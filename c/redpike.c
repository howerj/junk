/* Red Pike cipher source code from
 * https://en.wikipedia.org/wiki/Red_Pike_%28cipher%29 
 *
 * Pass in arrays of size 2, or a 64 bit value, key size is 64 bits,
 * block size is 64 bits */

#include <stdint.h>

typedef uint32_t word;

#define CONST 0x9E3779B9
#define ROUNDS 16

#define ROTL(X, R) (((X) << ((R) & 31)) | ((X) >> (32 - ((R) & 31))))
#define ROTR(X, R) (((X) >> ((R) & 31)) | ((X) << (32 - ((R) & 31))))

void encrypt(word * x, const word * k)
{
	unsigned int i;
	word rk0 = k[0];
	word rk1 = k[1];

	for (i = 0; i < ROUNDS; i++) {
		rk0 += CONST;
		rk1 -= CONST;

		x[0] ^= rk0;
		x[0] += x[1];
		x[0] = ROTL(x[0], x[1]);

		x[1] = ROTR(x[1], x[0]);
		x[1] -= x[0];
		x[1] ^= rk1;
	}

	rk0 = x[0];
	x[0] = x[1];
	x[1] = rk0;
}

void decrypt(word * x, const word * k)
{
	word dk[2] = {
		k[1] - CONST * (ROUNDS + 1),
		k[0] + CONST * (ROUNDS + 1)
	};

	encrypt(x, dk);
}
