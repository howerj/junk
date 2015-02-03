#ifndef TEA_H
#define TEA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void tea_encrypt(uint32_t data[2], uint32_t key[4]);
void tea_decrypt(uint32_t data[2], uint32_t key[4]);

#ifdef __cplusplus
}
#endif

#endif
