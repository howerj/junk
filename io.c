#include "libcompress.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

typedef int (*func_putc)(int c, io_t *);
typedef int (*func_getc)(io_t *);

static void fail_if(int test, const char *msg){
       if(test){
              if(errno && msg)
                      perror(msg);
              else if(msg)
                      fputs(msg, stderr);
              exit(-1);
       }
}

static void *calloc_or_fail(size_t n) {
        void *v;
	errno = 0;
        fail_if(!(v = calloc(n,1)), "calloc");
        return v;
}

struct io {
	func_putc put;
	func_getc get;

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
};

static io_t *new_io(void)
{
	return calloc_or_fail(sizeof(struct io));
}

void free_io(io_t *o)
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

io_t *io_file(FILE *f)
{
	assert(f);
	io_t *o = new_io();
	o->put = file_putc;
	o->get = file_getc;
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
	if(r != EOF)
		o->read++;
	return r;
}

int io_putc(int c, io_t *o)
{
	assert(o);
	int r = o->put(c, o);
	if(r == c)
		o->written++;
	return r;
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

io_t *io_string(unsigned ops, size_t size)
{
	io_t *o = new_io();
	o->readable = ops & IO_READ;
	o->writeable = ops & IO_WRITE;
	o->growable = ops & IO_REALLOC;
	o->we_own_the_string = 1;
	if(size)
		o->buf = calloc_or_fail(size+1); /**< NUL terminate just in case */
	o->max = size;
	o->put = string_putc;
	o->get = string_getc;
	return o;
}

io_t *io_string_external(unsigned ops, size_t size, uint8_t *initial)
{
	io_t *o = io_string(ops, size);
	o->we_own_the_string = 0;
	o->buf = initial;
	return o;
}

uint16_t io_fletcher16(uint8_t *data, size_t count)
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

