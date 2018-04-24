/** @file       rle.c
 *  @brief      a simple run length encoder
 *  @author     Richard James Howe.
 *  @copyright  Copyright 2016 Richard James Howe.
 *  @license    MIT (see https://opensource.org/licenses/MIT)
 *  @email      howe.r.j.89@gmail.com 
 *  @todo       turn into library (read/write to strings as well as files, man pages)
 * 
 * The encoder works on binary data, it encodes successive runs of
 * bytes as a length and the repeated byte up to runs of 127, or
 * a differing byte is found. This is then emitted as two bytes;
 * the length and the byte to repeat. Input data with no runs in
 * it is encoded as the length of the data that differs plus that
 * data, up to 128 bytes can be encoded this way.
 **/
#include "libcompress.h"
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum mode { InvalidMode, DecodeMode, EncodeMode };

enum errors { 
	Ok                  =   0, /* everything is groovy */
	ErrorHelp           =  -1, /* used to exit unsuccessfully after printing help message*/
	ErrorInEoF          =  -2, /* expected input but got an EOF */
	ErrorOutEoF         =  -3, /* expected to output bytes, but encountered and error */
	ErrorArg            =  -4, /* invalid command line args */
	ErrorFile           =  -5, /* could not open file, for whatever reason */
};

struct rle { /* contains bookkeeping information for encoding/decoding */
	enum mode mode;       /* mode of operation */
	jmp_buf *jb;          /* place to jump on error */
	io_t *in, *out;       /* input and output files */
};

static inline int may_fgetc(struct rle *r)
{ /* if the input ends now, that is okay */
	assert(r && r->in);
	return io_getc(r->in);
}

static inline int must_fgetc(struct rle *r)
{ /* we expect more input, otherwise our output will be invalid */
	assert(r && r->in);
	errno = 0;
	int c = io_getc(r->in);
	if(c == EOF) {
		char *reason = errno ? strerror(errno) : "(unknown reason)";
		fprintf(stderr, "error: expected more input, %s\n", reason);
		longjmp(*r->jb, ErrorInEoF);
	}
	return c;
}

static inline int must_fputc(struct rle *r, int c)
{ /* we must be able to output a character, otherwise our output is invalid */
	assert(r && r->out);
	errno = 0;
	if(c != io_putc(c, r->out)) {
		fprintf(stderr, "error: could not write '%d' to output, %s\n", c, io_strerror());
		longjmp(*r->jb, ErrorOutEoF);
	}
	return c;
}

static inline size_t must_block_io(struct rle *r, void *p, size_t size, char rw)
{ /* the block must be written or read, otherwise something has gone wrong */
	errno = 0;
	assert(r && p && ((rw == 'w') || (rw == 'r')));
	io_t *file = rw == 'w' ? 
		r->out : 
		r->in;
	size_t ret = rw == 'w' ? 
		io_write(p, size, file) : 
		io_read (p, size, file);
	if(ret != size) {
		char *type   = rw == 'w' ? "write" : "read";
		fprintf(stderr, "error: could not %s block, %s\n", type, io_strerror());
		longjmp(*r->jb, ErrorOutEoF);
	}
	return ret;
}

static inline void must_encode_buf(struct rle *r, uint8_t *buf, int *idx)
{ /* the output block must be encoded, this is a run of literal text */
	must_fputc(r, (*idx)+128);
	must_block_io(r, buf, *idx, 'w');
	*idx = 0;
}

static void encode(struct rle *r)
{ 
	uint8_t buf[128]; /* buffer to store data with no runs */
	int idx = 0;
	int prev = EOF; /* no previously read in char can be EOF */
	for(int c; (c = may_fgetc(r)) != EOF; prev = c) {
		if(c == prev) { /* encode runs of data */
			int j;  /* count of runs */
			if(idx > 1) /* output any existing data */
				must_encode_buf(r, buf, &idx);
again:
			for(j = 0; (c = may_fgetc(r)) == prev && j < 128; j++)
				/*loop does everything*/;
			must_fputc(r, j);
			must_fputc(r, prev);
			if(c == EOF)
				goto end;
			if(c == prev && j == 128) /**/
				goto again;
		}
		buf[idx++] = c;
		if(idx == 127)
			must_encode_buf(r, buf, &idx);
		assert(idx < 127);
	}
end: /* no more input */
	if(idx) /* we might still have something in the buffer though */
		must_encode_buf(r, buf, &idx);
}

int run_length_encode(io_t *in, io_t *out)
{
	assert(in && out);
	jmp_buf jb;
	struct rle r = { EncodeMode, &jb, in, out};
	int rval;
	if((rval = setjmp(jb)))
		return rval;
	encode(&r);
	return Ok;
}

static void decode(struct rle *r)
{ /* RLE decoder, the function is quite simple */
	assert(r);
	for(int c, count; (c = may_fgetc(r)) != EOF;) {
		if(c > 128) { /* process run of literal data */
			count = c - 128;
			for(int i = 0; i < count; i++) {
				c = must_fgetc(r);
				must_fputc(r, c);
			}
		} else { /* process repeated byte */
			count = c + 1;
			c = must_fgetc(r);
			for(int i = 0; i < count; i++)
				must_fputc(r, c);
		}
	}
}

int run_length_decode(io_t *in, io_t *out)
{
	assert(in && out);
	jmp_buf jb;
	struct rle r = { DecodeMode, &jb, in, out};
	int rval;
	if((rval = setjmp(jb)))
		return rval;
	decode(&r);
	return Ok;
}

static void help(void)
{
	static const char help_text[] =
"\nRun Length Encoder and Decoder\n\n\
	-e\tencode\n\
	-d\tdecode (mutually exclusive with '-e')\n\
	-h\tprint help and exit unsuccessfully\n\
	-\tstop processing command line options\n\n\
The file parameters are optional, with the possible combinations:\n\
\n\
\t0 files specified:\n\t\tinput:  stdin   \n\t\toutput: stdout\n\
\t1 file  specified:\n\t\tinput:  1st file\n\t\toutput: stdout\n\
\t2 files specified:\n\t\tinput:  1st file\n\t\toutput: 2nd file\n\
\n";
	fputs(help_text, stderr);
}

static void usage(char *name)
{
	assert(name);
	fprintf(stderr, "usage %s [-(e|d)] [-h] [file.in] [file.out]\n", name);
}

int main_rle(int argc, char **argv)
{ 
	FILE *infile  = stdin, *outfile  = stdout; /* default to using standard streams */
	io_t *in = NULL, *out = NULL;
	char *cin = NULL,  *cout = NULL;
	int r = ErrorArg, i;
	enum mode mode = InvalidMode;
	for(i = 1; i < argc && argv[i][0] == '-'; i++) {
		switch(argv[i][1]) {
		case '\0': goto done;
		case 'h':  usage(argv[0]); help(); return ErrorHelp;
		case 'e':  if(mode) goto fail; mode = EncodeMode; break;
		case 'd':  if(mode) goto fail; mode = DecodeMode; break;
		default:   goto fail;
		}
	}
done:
	if(i < argc) /* first file argument is input file, if argument exists */
		cin  = argv[i++];
	if(i < argc) /* second file argument is output file, if argument exists */
		cout = argv[i++];
	if(i < argc) /* there should be no more arguments after this */
		goto fail;
	if(cin)
		infile  = io_fopen_or_fail(cin,  "rb");
	if(cout)
		outfile = io_fopen_or_fail(cout, "wb");

	in  = io_file(infile);
	out = io_file(outfile);

	if(mode == InvalidMode) /*default to encode*/
		mode = EncodeMode;
	if(mode == EncodeMode)
		r = run_length_encode(in, out);
	if(mode == DecodeMode)
		r = run_length_decode(in, out);
	io_free(in);
	io_free(out);
	return r;
fail:
	usage(argv[0]);
	return ErrorArg;
}

#ifdef RLE_MAIN 
int main(int argc, char *argv[])
{
	return main_rle(argc, argv);
}
#endif
