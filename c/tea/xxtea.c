/* Source <https://en.wikipedia.org/wiki/XXTEA>
 * The PDF describing the original algorithm should be in the
 * same repo as this code.
 */
#include "xxtea.h"

#define DELTA (0x9e3779b9)
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))
 
void xxtea_encrypt(unsigned vlen, uint32_t *v, uint32_t const key[4]){
        uint32_t y, z, sum;
        unsigned p, rounds, e;
        rounds = 6 + 52 / vlen;
        sum = 0;
        z = v[vlen - 1];
        do {
                sum += DELTA;
                e = (sum >> 2) & 3;
                for (p = 0; p < vlen - 1; p++) {
                        y = v[p + 1];
                        z = v[p] += MX;
                }
                y = v[0];
                z = v[vlen - 1] += MX;
        } while (--rounds);
}

void xxtea_decrypt(unsigned vlen, uint32_t *v, uint32_t const key[4])
{
        uint32_t y, z, sum;
        unsigned p, rounds, e;
        rounds = 6 + 52 / vlen;
        sum = rounds * DELTA;
        y = v[0];
        do {
                e = (sum >> 2) & 3;
                for (p = vlen - 1; p > 0; p--) {
                        z = v[p - 1];
                        y = v[p] -= MX;
                }
                z = v[vlen - 1];
                y = v[0] -= MX;
                sum -= DELTA;
        } while (--rounds);
}

