#ifndef  COMPRESS_H
#define COMPRESS_H

#include <stddef.h>
#include <stdint.h>

size_t compress(void *s_start, void *d_start, size_t s_len);
size_t decompress(void *s_start, void *d_start, size_t s_len, size_t d_len);
uint32_t checksum32(void *cp_arg, size_t length);

#endif
