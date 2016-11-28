/** @file       rle.c
 *  @brief      a simple run length encoder
 *  @author     Richard James Howe.
 *  @copyright  Copyright 2016 Richard James Howe.
 *  @license    MIT (see https://opensource.org/licenses/MIT)
 *  @email      howe.r.j.89@gmail.com 
 *  @todo       add checksum, length and whether encoded succeeded (size decreased) to header
 *  @todo       turn into library (read/write to strings as well as files, man pages)
 * 
 * The encoder works on binary data, it encodes successive runs of
 * bytes as a length and the repeated byte up to runs of 127, or
 * a differing byte is found. This is then emitted as two bytes;
 * the length and the byte to repeat. Input data with no runs in
 * it is encoded as the length of the data that differs plus that
 * data, up to 128 bytes can be encoded this way.
 **/
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define  VERSION            (0x2) /* version of binary output format */
#define  INVALID_HASH       (0xFFFFu) /* fletcher16 hash can never be this value */
#define  INVALID_SIZE_FIELD (0x0) /* the size field, that follows, is invalid */
#define  INVALID_HASH_FIELD (0x0) /* the hash field, that follows, is invalid */

enum mode { InvalidMode, DecodeMode, EncodeMode };

enum errors { 
	Ok                  =   0, /* everything is groovy */
	ErrorHelp           =  -1, /* used to exit unsuccessfully after printing help message*/
	ErrorInEoF          =  -2, /* expected input but got an EOF */
	ErrorOutEoF         =  -3, /* expected to output bytes, but encountered and error */
	ErrorArg            =  -4, /* invalid command line args */
	ErrorFile           =  -5, /* could not open file, for whatever reason */
	ErrorInvalidHeader  =  -6  /* invalid header */
};

enum header { /* */
	eMagic0,     eMagic1,     eMagic2,     eMagic3,
	eVersion,    eReserved0,  eReserved1,  eEndMagic,
	eSizeValid,  eHashValid,  eHash0,      eHash1,
	eReserved2,  eReserved3,  eReserved4,  eReserved5,
	eSize0,      eSize1,      eSize2,      eSize3,   
	eSize4,      eSize5,      eSize6,      eSize7
};

static uint8_t header[24] = { 
	0xFF,    'R', 'L', 'E',   /* magic numbers to identify file type */
	VERSION,  0,   0,   0xFF, /* file version number, reserved bytes, end byte */
	INVALID_SIZE_FIELD, INVALID_HASH_FIELD, (uint8_t)INVALID_HASH, (uint8_t)(INVALID_HASH >> 8),
	0,        0,   0,   0,    /* reserved */

	0,        0,   0,   0,    /* size */
	0,        0,   0,   0,    /* size */
};

struct rle { /* contains bookkeeping information for encoding/decoding */
	enum mode mode;       /* mode of operation */
	jmp_buf *jb;          /* place to jump on error */
	FILE *in, *out;       /* input and output files */
	uint64_t read, wrote; /* bytes read in and wrote */
	uint16_t hash;        /* hash of original data */
};

static inline uint16_t fletcher16(uint8_t *data, size_t count)
{ /* https://en.wikipedia.org/wiki/Fletcher%27s_checksum */
	uint16_t x = 0, y = 0;
	size_t i;
	/* this function needs splitting into a start function,
	 * a hash function and an end function */
	for(i = 0; i < count; i++) {
		x = (x + data[i]) & 255;
		y = (y + x) & 255;
	}
	return (y << 8) | x;
}

static inline int may_fgetc(struct rle *r)
{ /* if the input ends now, that is okay */
	assert(r && r->in);
	int rval = fgetc(r->in);
	r->read += rval != EOF;
	return rval;
}

static inline int expect_fgetc(struct rle *r)
{ /* we expect more input, otherwise our output will be invalid */
	assert(r && r->in);
	errno = 0;
	int c = fgetc(r->in);
	if(c == EOF) {
		char *reason = errno ? strerror(errno) : "(unknown reason)";
		fprintf(stderr, "error: expected more input, %s\n", reason);
		longjmp(*r->jb, ErrorInEoF);
	}
	r->read++;
	return c;
}

static inline int must_fputc(struct rle *r, int c)
{ /* we must be able to output a character, otherwise our output is invalid */
	assert(r && r->out);
	errno = 0;
	if(c != fputc(c, r->out)) {
		char *reason = errno ? strerror(errno) : "(unknown reason)";
		fprintf(stderr, "error: could not write '%d' to output, %s\n", c, reason);
		longjmp(*r->jb, ErrorOutEoF);
	}
	r->wrote++;
	return c;
}

static inline void increment(struct rle *r, char rw, size_t size)
{ /* increment the correct counter for reading or writing */
	assert(r && ((rw == 'w') || (rw == 'r')));
	if(rw == 'w')
		r->wrote += size;
	else
		r->read  += size;
}

static inline size_t must_block_io(struct rle *r, void *p, size_t size, char rw)
{ /* the block must be written or read, otherwise something has gone wrong */
	errno = 0;
	assert(r && p && ((rw == 'w') || (rw == 'r')));
	FILE *file = rw == 'w' ? 
		r->out : 
		r->in;
	size_t ret = rw == 'w' ? 
		fwrite(p, 1, size, file) : 
		fread (p, 1, size, file);
	increment(r, rw, ret);
	if(ret != size) {
		char *reason = errno ? strerror(errno) : "(unknown reason)";
		char *type   = rw == 'w' ? "write" : "read";
		fprintf(stderr, "error: could not %s block, %s\n", type, reason);
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

static void print_results(int verbose, struct rle *r, int encode)
{
	if(!verbose)
		return;
	fputs(encode ? "encode:\n" : "decode:\n", stderr);
	fprintf(stderr, "\tread   %"PRIu64"\n", r->read);
	fprintf(stderr, "\twrote  %"PRIu64"\n", r->wrote);
	fprintf(stderr, "\thash   %"PRIu16"\n", r->hash);
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

int run_length_encoder(int headerless, int verbose, FILE *in, FILE *out)
{
	assert(in && out);
	jmp_buf jb;
	struct rle r = { EncodeMode, &jb, in, out, 0, 0, 0};
	int rval;
	if((rval = setjmp(jb)))
		return rval;
	if(!headerless)
		must_block_io(&r, header, sizeof(header), 'w');
	encode(&r);
	print_results(verbose, &r, 1);
	return Ok;
}

static void decode(struct rle *r)
{ /* RLE decoder, the function is quite simple */
	assert(r);
	for(int c, count; (c = may_fgetc(r)) != EOF;) {
		if(c > 128) { /* process run of literal data */
			count = c - 128;
			for(int i = 0; i < count; i++) {
				c = expect_fgetc(r);
				must_fputc(r, c);
			}
		} else { /* process repeated byte */
			count = c + 1;
			c = expect_fgetc(r);
			for(int i = 0; i < count; i++)
				must_fputc(r, c);
		}
	}
}

int run_length_decoder(int headerless, int verbose, FILE *in, FILE *out)
{
	assert(in && out);
	jmp_buf jb;
	struct rle r = { DecodeMode, &jb, in, out, 0, 0, 0};
	char head[sizeof(header)] = {0};
	int rval;
	if((rval = setjmp(jb)))
		return rval;
	if(!headerless) {
		must_block_io(&r, head, sizeof(head), 'r');
		if(memcmp(head, header, eEndMagic+1)) {
			fprintf(stderr, "error: invalid header\n");
			return ErrorInvalidHeader;
		}
	}
	decode(&r);
	print_results(verbose, &r, 0);
	return Ok;
}


