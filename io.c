#include "libcompress.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/**@todo add io_printf, io_scanf */

typedef int (*func_putc)(int c, io_t *);
typedef int (*func_getc)(io_t *);
typedef int (*func_flush)(io_t *);

struct io {
	func_putc put;
	func_getc get;
	func_flush flush;

	FILE *file;

	uint8_t *buf;
	size_t rindex; /**< read index */
	size_t windex; /**< write index */
	size_t max;    /**< maximum chars in buf */
	unsigned we_own_the_string : 1,
		 readable : 1,
		 writeable : 1,
		 growable : 1;

	size_t written;
	size_t read;

	fletcher16_t rhash;
	fletcher16_t whash;
};

static void fail_if(int test, const char *msg){
       if(test){
              if(errno && msg)
                      perror(msg);
              else if(msg)
                      fputs(msg, stderr);
              exit(EXIT_FAILURE);
       }
}

void *io_calloc_or_fail(size_t n) 
{
        void *v;
	errno = 0;
        fail_if(!(v = calloc(n, 1)), "calloc");
        return v;
}

static io_t *new_io(void)
{
	io_t *r = io_calloc_or_fail(sizeof(*r));
	r->rhash = io_fletcher16_start();
	r->whash = io_fletcher16_start();
	return r;
}

void io_free(io_t *o)
{
	assert(o && (!!(o->file) ^ !!(o->buf)));
	if(o->buf && o->we_own_the_string)
		free(o->buf);
	if(o->file)
		fclose(o->file);
	memset(o, 0, sizeof(*o));
	free(o);
}

static int file_putc(int c, io_t *o)
{
	assert(o && o->file);
	return fputc(c, o->file);
}

static int file_getc(io_t *o)
{
	assert(o && o->file);
	return fgetc(o->file);
}

static int file_flush(io_t *o)
{
	assert(o && o->file);
	return fflush(o->file);
}

io_t *io_file(FILE *f)
{
	assert(f);
	io_t *o = new_io();
	o->put = file_putc;
	o->get = file_getc;
	o->flush = file_flush;
	o->file = f;
	return o;
}

uint8_t *io_get_string(io_t *o)
{
	assert(o && (!!(o->file) ^ !!(o->buf)));
	return o->buf;
}

int io_getc(io_t *o)
{
	assert(o);
	int r = o->get(o);
	if(r != EOF) {
		o->read++;
		io_fletcher16_update(&o->rhash, r);
	}
	return r;
}

int io_putc(int c, io_t *o)
{
	assert(o);
	int r = o->put(c, o);
	if(r == c) {
		o->written++;
		if(c != EOF)
			io_fletcher16_update(&o->whash, c);
	}
	return r;
}

int io_flush(io_t *o)
{
	assert(o);
	return o->flush(o);
}

/**
@note obviously this is super inefficient, io_putc and io_getc
should be rewritten to be in terms of io_write and io_read, not
this way around
**/

size_t io_read(uint8_t *buf, size_t size, io_t *o)
{
	size_t i = 0;
	int c = 0;
	func_getc get = o->get;
	for(i = 0; i < size; i++)
		if(EOF == (c = get(o)))
			return i;
		else
			buf[i] = c;
	return i;
}

size_t io_write(uint8_t *buf, size_t size, io_t *o)
{
	size_t i = 0;
	func_putc put = o->put;
	for(i = 0; i < size; i++)
		if(buf[i] != put(buf[i], o))
			return i;
	return i;
}

const char *io_strerror(void)
{
	static const char *unknown = "unknown reason";
	const char *r = errno ? strerror(errno) : unknown;
	if(!r) 
		r = unknown;
	return r;
}

int io_must_putc(int c, io_t *o)
{
	int r;
	assert(o);
	errno = 0;
	r = o->put(c, o);
	if(r != c) {
		fprintf(stderr, "failed to put %c to %p: %s\n", c, o, io_strerror());
		exit(EXIT_FAILURE);
	}
	return r;
}

int io_must_getc(io_t *o)
{
	int r;
	assert(o);
	errno = 0;
	r = o->get(o);
	if(r == EOF) {
		fprintf(stderr, "failed to get character from %p: %s\n", o, io_strerror());
		exit(EXIT_FAILURE);
	}
	return r;
}

size_t io_get_chars_written(io_t *o)
{
	return o->written;
}

size_t io_get_chars_read(io_t *o)
{
	return o->read;
}

uint16_t io_get_hash_written(io_t *o)
{
	return io_fletcher16_end(&o->whash);
}

uint16_t io_get_hash_read(io_t *o)
{
	return io_fletcher16_end(&o->rhash);
}


static int string_putc(int c, io_t *o)
{
	uint8_t *p;
	size_t maxt;
	assert(o);
	if(!o->writeable)
		return EOF;
	if ((o->windex >= (o->max - 1)) && o->growable) { /*grow the "file" */
		maxt = (o->max + 1) * 2;
		if (maxt < o->max)	/*overflow */
			return EOF;
		if (!(p = realloc(o->buf, maxt)))
			return EOF;
		o->max = maxt;
		memset(p + o->windex, 0, maxt - o->windex);
		o->buf = p;
	}
	if(o->windex < o->max) {
		o->buf[o->windex++] = c;
		return c;
	}
	return EOF;
}

static int string_getc(io_t *o)
{
	assert(o);
	if(!o->readable)
		return EOF;
	/**@todo fix this so it does not go past windex if writable */
	return o->rindex < o->max ? o->buf[o->rindex++] : EOF;
}

static int string_flush(io_t *o)
{
	assert(o);
	return 0;
}

static io_t *io_string_allocator(unsigned ops, size_t size, int allocate)
{
	io_t *o = new_io();
	o->readable  = (ops & IO_READ)    >> 0;
	o->writeable = (ops & IO_WRITE)   >> 1;
	o->growable  = (ops & IO_REALLOC) >> 2;
	o->we_own_the_string = 1;
	if(allocate)
		o->buf = io_calloc_or_fail(size+1); /**< NUL terminate just in case */
	o->max = size;
	o->put = string_putc;
	o->get = string_getc;
	o->flush = string_flush;
	return o;
}

io_t *io_string(unsigned ops, size_t size)
{
	return io_string_allocator(ops, size, 1);
}

io_t *io_string_external(unsigned ops, size_t size, uint8_t *initial)
{
	io_t *o = io_string_allocator(ops, size, 0);
	o->we_own_the_string = 0;
	o->buf = initial;
	return o;
}

fletcher16_t io_fletcher16_start(void)
{
	fletcher16_t f = { 0, 0 };
	return f;
}

void io_fletcher16_update(fletcher16_t *f, uint8_t b)
{
	f->x = (f->x + b)    & 255;
	f->y = (f->y + f->x) & 255;
}

uint16_t io_fletcher16_end(fletcher16_t *f)
{
	return (f->y << 8) | f->x;
}

uint16_t io_fletcher16_block(uint8_t *data, size_t count)
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

