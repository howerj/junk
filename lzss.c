/** From http://oku.edu.mie-u.ac.jp/~okumura/compression/lzss.c
 * @todo Turn into library
 * @todo Document format and how encoding/decoding works
 * @todo Correct types (use 'int' less)
 * @todo Add header? Also add statistics grabbing functions
 * @todo This implementation is slow, it needs speeding up *a lot*, there are
 * two ways to do this, increase the speed of the search (highest priority) and
 * output speed up the output.
 * LZSS encoder-decoder (Haruhiko Okumura; public domain) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libcompress.h"

#define EI 11              /* typically 10..13 */
#define EJ  4              /* typically 4..5 */
#define P   1              /* If match length <= P then output one character */
#define N  (1 << EI)       /* buffer size */
#define F  ((1 << EJ) + 1) /* lookahead buffer size */

struct lzss {
	io_t *in;
	io_t *out;
	unsigned output_bit_buffer;
	unsigned output_bit_mask;
	unsigned input_bit_buffer; 
	unsigned input_bit_mask;
	unsigned char buffer[N * 2];
};

void lzss_free(lzss_t *l)
{
	memset(l, 0, sizeof(*l));
	free(l);
}

lzss_t *lzss_new(io_t *in, io_t *out)
{
	lzss_t *l = io_calloc_or_fail(sizeof(*l));
	l->in = in;
	l->out = out;
	l->output_bit_mask = 128;
	return l;
}

static int putbit(lzss_t *l, unsigned put_one)
{
	if(put_one)
		l->output_bit_buffer |= l->output_bit_mask;
	if ((l->output_bit_mask >>= 1) == 0) {
		if (io_putc(l->output_bit_buffer, l->out) == EOF)
			return -1;
		l->output_bit_buffer = 0;
		l->output_bit_mask = 128;
	}
	return 0;
}

static int flush_output_bit_buffer(lzss_t *l)
{
	if (l->output_bit_mask != 128) {
		if (io_putc(l->output_bit_buffer, l->out) == EOF)
			return -1;
	}
	return 0;
}

static int output(lzss_t *l, unsigned mask, unsigned c)
{ 
	while(mask >>= 1)
		if(putbit(l, c & mask) < 0)
			return -1;
	return 0;
}

static int output1(lzss_t *l, unsigned c)
{
	if(putbit(l, 1) < 0)
		return -1;
	if(output(l, 256, c) < 0)
		return -1;
	return 0;
}

static int output2(lzss_t *l, unsigned x, unsigned y)
{
	if(putbit(l, 0) < 0)
		return -1;
	if(output(l, N, x) < 0)
		return -1;
	if(output(l, 1 << EJ, y) < 0)
		return -1;
	return 0;
}

int lzss_encode(lzss_t *l)
{
	int i, j, f1, x, y, r, s, bufferend, c;

	for (i = 0; i < N - F; i++)
		l->buffer[i] = ' ';
	for (i = N - F; i < N * 2; i++) {
		if ((c = io_getc(l->in)) == EOF)
			break;
		l->buffer[i] = c;
	}
	bufferend = i;
	r = N - F;
	s = 0;
	while (r < bufferend) {
		f1 = (F <= bufferend - r) ? F : bufferend - r;
		x = 0;
		y = 1;
		c = l->buffer[r];
		/* search for match
		 * @todo speed this up, this slows everything down */
		for (i = r - 1; i >= s; i--)
			if (l->buffer[i] == c) {
				for (j = 1; j < f1; j++)
					if (l->buffer[i + j] != l->buffer[r + j])
						break;
				if (j > y) {
					x = i;
					y = j;
				}
			}
		if (y <= P) {
			y = 1;
			if(output1(l, c) < 0)
				return -1;
		} else {
			if(output2(l, x & (N - 1), y - 2) < 0)
				return -1;
		}
		r += y;
		s += y;
		if (r >= N * 2 - F) { /* refill buffer */
			memcpy(l->buffer, l->buffer + N, N);
			bufferend -= N;
			r -= N;
			s -= N;
			while (bufferend < N * 2) { /**@todo replace with io_read*/
				if ((c = io_getc(l->in)) == EOF)
					break;
				l->buffer[bufferend++] = c;
			}
		}
	}
	if(flush_output_bit_buffer(l) < 0)
		return -1;
	return 0;
}

static int getbit(lzss_t *l, unsigned n)
{ /* get n bits */
	unsigned i, x;

	x = 0;
	for (i = 0; i < n; i++) {
		if (l->input_bit_mask == 0) {
			int c;
			if ((c = io_getc(l->in)) == EOF)
				return EOF;
			l->input_bit_buffer = c;
			l->input_bit_mask = 128;
		}
		x <<= 1;
		if (l->input_bit_buffer & l->input_bit_mask)
			x++;
		l->input_bit_mask >>= 1;
	}
	return x;
}

int lzss_decode(lzss_t *l)
{
	int i, j, k, r, c;

	memset(l->buffer, ' ', N - F);
	r = N - F;
	while ((c = getbit(l, 1)) != EOF) {
		if (c) {
			if ((c = getbit(l, 8)) == EOF)
				break;
			if(io_putc(c, l->out) < 0)
				return -1;
			l->buffer[r++] = c;
			r &= (N - 1);
		} else {
			if ((i = getbit(l, EI)) == EOF)
				break;
			if ((j = getbit(l, EJ)) == EOF)
				break;
			for (k = 0; k <= j + 1; k++) {
				c = l->buffer[(i + k) & (N - 1)];
				if(io_putc(c, l->out) < 0)
					return -1;
				l->buffer[r++] = c;
				r &= (N - 1);
			}
		}
	}
	return 0;
}

int main_lzss(int argc, char *argv[])
{
	int encode;
	char *s;
	lzss_t *l;
	FILE *infile = NULL, *outfile = NULL;
	size_t read = 0, written = 0;
	io_t *in, *out;

	if (argc != 4) {
		fprintf(stderr, "Usage: lzss e/d infile outfile\n\te = encode\td = decode\n");
		return 1;
	}
	s = argv[1];
	if (s[1] == 0 && (*s == 'd' || *s == 'D' || *s == 'e' || *s == 'E')) {
		encode = (*s == 'e' || *s == 'E');
	} else {
		fprintf(stderr, "? %s\n", s);
		return 1;
	}
	if ((infile = fopen(argv[2], "rb")) == NULL) {
		fprintf(stderr, "? %s\n", argv[2]);
		return 1;
	}
	if ((outfile = fopen(argv[3], "wb")) == NULL) {
		fprintf(stderr, "? %s\n", argv[3]);
		return 1;
	}
	in  = io_file(infile);
	out = io_file(outfile);
	
	l = lzss_new(in, out);

	if (encode)
		lzss_encode(l);
	else
		lzss_decode(l);
	lzss_free(l);

	read = io_get_chars_read(in);
	written = io_get_chars_written(out);

	fprintf(stderr, "%s\ninput:   %zu bytes\noutput:  %zu bytes\nratio:   %.2lf%%\n", 
		encode ? "compressing" : "decompressing",
		read, 
		written, read ? (double)(written * 100ull) / read: 0.0);

	io_free(in);
	io_free(out);
	return 0;
}

#ifdef LZSS_MAIN
int main(int argc, char *argv[])
{
	return main_lzss(argc, argv);
}
#endif
