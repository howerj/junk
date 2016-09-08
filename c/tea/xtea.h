#ifndef XTEA_H
#define XTEA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void xtea_encrypt(unsigned num_rounds, uint32_t v[2], uint32_t const key[4]);
void xtea_decrypt(unsigned num_rounds, uint32_t v[2], uint32_t const key[4]);

#ifdef __cplusplus
}
#endif

#endif
