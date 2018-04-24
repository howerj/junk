#ifndef LIBCOMPRESS_H
#define LIBCOMPRESS_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { 
	IO_READ    = 0x1,
	IO_WRITE   = 0x2,
	IO_RW      = 0x3, /**< IO_READ | IO_WRITE */
	IO_REALLOC = 0x4,
} io_string_operations;

struct io;
typedef struct io io_t;

typedef struct {
	uint16_t x;
	uint16_t y;
} fletcher16_t;

FILE *io_fopen_or_fail(const char *file, const char *mode);
void *io_calloc_or_fail(size_t n);
void io_free(io_t *o);
io_t *io_file(FILE *f);
int io_getc(io_t *o);
int io_putc(int c, io_t *o);
int io_must_getc(io_t *o);
int io_must_putc(int c, io_t *o);
int io_flush(io_t *o);
io_t *io_string(unsigned ops, size_t size);
const char *io_strerror(void);
uint8_t *io_get_string(io_t *o);
io_t *io_string_external(unsigned ops, size_t size, uint8_t *initial);
io_t *io_string(unsigned ops, size_t size);
uint16_t io_fletcher16_block(uint8_t *data, size_t count);
size_t io_get_chars_written(io_t *o);
size_t io_get_chars_read(io_t *o);
fletcher16_t io_fletcher16_start(void);
void io_fletcher16_update(fletcher16_t *f, uint8_t b);
uint16_t io_fletcher16_end(fletcher16_t *f);
uint16_t io_get_hash_written(io_t *o);
uint16_t io_get_hash_read(io_t *o);
size_t io_read(uint8_t *buf, size_t size, io_t *o);
size_t io_write(uint8_t *buf, size_t size, io_t *o);
void io_rewind(io_t *o);

/**@todo versions should be provided for strings and for files, not just io_t types */

int lzss_encode(io_t *in, io_t *out); /* returns negative on failure to encode */
int lzss_decode(io_t *in, io_t *out); /* returns negative on failure to decode */
int main_lzss(int argc, char **argv);

int run_length_encode(io_t *in, io_t *out);
int run_length_decode(io_t *in, io_t *out);
int main_rle(int argc, char **argv);

#ifdef __cplusplus
}
#endif
#endif 
