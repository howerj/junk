/** From http://oku.edu.mie-u.ac.jp/~okumura/compression/lzss.c
 * @todo Turn into library
 * @todo Document format and how encoding/decoding works
 * @todo Correct types (use 'int' less)
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
	unsigned output_bit_buffer;
	unsigned output_bit_mask;
	int input_bit_buffer; 
	int input_bit_mask;

	unsigned long long codecount;
	unsigned long long textcount;
	unsigned char buffer[N * 2];
	FILE *infile;
	FILE *outfile;
};

void lzss_free(lzss_t *l)
{
	memset(l, 0, sizeof(*l));
	free(l);
}

lzss_t *lzss_new(FILE *infile, FILE *outfile)
{
	lzss_t *l = io_calloc_or_fail(sizeof(*l));
	l->infile = infile;
	l->outfile = outfile;
	l->output_bit_mask = 128;
	return l;
}

static void error(void)
{
	fprintf(stderr, "Output error\n");
	exit(EXIT_FAILURE);
}

static void putbit(lzss_t *l, unsigned put_one)
{
	if(put_one)
		l->output_bit_buffer |= l->output_bit_mask;
	if ((l->output_bit_mask >>= 1) == 0) {
		if (fputc(l->output_bit_buffer, l->outfile) == EOF)
			error();
		l->output_bit_buffer = 0;
		l->output_bit_mask = 128;
		l->codecount++;
	}
}

static void flush_output_bit_buffer(lzss_t *l)
{
	if (l->output_bit_mask != 128) {
		if (fputc(l->output_bit_buffer, l->outfile) == EOF)
			error();
		l->codecount++;
	}
}

static void output1(lzss_t *l, unsigned c)
{
	unsigned mask;

	putbit(l, 1);
	mask = 256;
	while (mask >>= 1)
		putbit(l, c & mask);
}

static void output2(lzss_t *l, unsigned x, unsigned y)
{
	unsigned mask;
	putbit(l, 0);
	mask = N;
	while (mask >>= 1)
		putbit(l, x & mask);
	mask = (1 << EJ);
	while (mask >>= 1)
		putbit(l, y & mask);
}

void lzss_encode(lzss_t *l)
{
	int i, j, f1, x, y, r, s, bufferend, c;

	for (i = 0; i < N - F; i++)
		l->buffer[i] = ' ';
	for (i = N - F; i < N * 2; i++) {
		if ((c = fgetc(l->infile)) == EOF)
			break;
		l->buffer[i] = c;
		l->textcount++;
	}
	bufferend = i;
	r = N - F;
	s = 0;
	while (r < bufferend) {
		f1 = (F <= bufferend - r) ? F : bufferend - r;
		x = 0;
		y = 1;
		c = l->buffer[r];
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
			output1(l, c);
		} else
			output2(l, x & (N - 1), y - 2);
		r += y;
		s += y;
		if (r >= N * 2 - F) {
			for (i = 0; i < N; i++)
				l->buffer[i] = l->buffer[i + N];
			bufferend -= N;
			r -= N;
			s -= N;
			while (bufferend < N * 2) {
				if ((c = fgetc(l->infile)) == EOF)
					break;
				l->buffer[bufferend++] = c;
				l->textcount++;
			}
		}
	}
	flush_output_bit_buffer(l);
	fprintf(stderr, "text:  %lld bytes\ncode:  %lld bytes (%.2lf%%)\n", 
		l->textcount,
		l->codecount, (double)(l->codecount * 100ull) / l->textcount);
}

static int getbit(lzss_t *l, unsigned n)
{ /* get n bits */
	unsigned i, x;

	x = 0;
	for (i = 0; i < n; i++) {
		if (l->input_bit_mask == 0) {
			if ((l->input_bit_buffer = fgetc(l->infile)) == EOF)
				return EOF;
			l->input_bit_mask = 128;
		}
		x <<= 1;
		if (l->input_bit_buffer & l->input_bit_mask)
			x++;
		l->input_bit_mask >>= 1;
	}
	return x;
}

void lzss_decode(lzss_t *l)
{
	int i, j, k, r, c;

	for (i = 0; i < N - F; i++)
		l->buffer[i] = ' ';
	r = N - F;
	while ((c = getbit(l, 1)) != EOF) {
		if (c) {
			if ((c = getbit(l, 8)) == EOF)
				break;
			fputc(c, l->outfile);
			l->buffer[r++] = c;
			r &= (N - 1);
		} else {
			if ((i = getbit(l, EI)) == EOF)
				break;
			if ((j = getbit(l, EJ)) == EOF)
				break;
			for (k = 0; k <= j + 1; k++) {
				c = l->buffer[(i + k) & (N - 1)];
				fputc(c, l->outfile);
				l->buffer[r++] = c;
				r &= (N - 1);
			}
		}
	}
}

int main_lzss(int argc, char *argv[])
{
	int enc;
	char *s;
	lzss_t *l;
	FILE *infile = NULL, *outfile = NULL;

	if (argc != 4) {
		fprintf(stderr, "Usage: lzss e/d infile outfile\n\te = encode\td = decode\n");
		return 1;
	}
	s = argv[1];
	if (s[1] == 0 && (*s == 'd' || *s == 'D' || *s == 'e' || *s == 'E'))
		enc = (*s == 'e' || *s == 'E');
	else {
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
	
	l = lzss_new(infile, outfile);

	if (enc)
		lzss_encode(l);
	else
		lzss_decode(l);
	fclose(infile);
	fclose(outfile);
	return 0;
}

#ifdef LZSS_MAIN
int main(int argc, char *argv[])
{
	return main_lzss(argc, argv);
}
#endif
