#ifndef XXTEA_H
#define XXTEA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void xxtea_encrypt(unsigned vlen, uint32_t *v, uint32_t const key[4]);
void xxtea_decrypt(unsigned vlen, uint32_t *v, uint32_t const key[4]);

#ifdef __cplusplus
}
#endif

#endif
