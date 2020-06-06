/**@author Richard James Howe
 * @license MIT */
#include "system.h"

typedef unsigned long bitmap_unit_t;
#define BITS (sizeof(bitmap_unit_t)*CHAR_BIT)
#define MASK (BITS-1)

typedef struct {
	size_t bits;
	bitmap_unit_t map[];
} bitmap_t;

/**@todo assert bit index in range */

size_t bitmap_units(size_t bits) {
	return bits/BITS + !!(bits & MASK);
}

size_t bitmap_bits(bitmap_t *b)
{
	assert(b);
	return b->bits;
}

size_t bitmap_sizeof(bitmap_t *b) {
	assert(b);
	return sizeof(*b) + bitmap_units(b->bits)*sizeof(bitmap_unit_t);
}

bitmap_t *bitmap_new(size_t bits) {
	size_t length = bitmap_units(bits)*sizeof(bitmap_unit_t);
	/**@todo detect overflow */
	bitmap_t *r = calloc(sizeof(bitmap_t) + length, 1);
	if(!r)
		return NULL;
	r->bits = bits;
	return r;
}

bitmap_t *bitmap_copy(bitmap_t *b) {
	assert(b);
	bitmap_t *r = bitmap_new(b->bits);
	if(!r)
		return NULL;
	return memcpy(r, b, bitmap_sizeof(b));
}

void bitmap_free(bitmap_t *b) {
	free(b);
}

void bitmap_set(bitmap_t *b, size_t bit) {
	assert(b);
	b->map[bit/BITS] |=  (1u << (bit & MASK));
}

void bitmap_clear(bitmap_t *b, size_t bit) {
	assert(b);
	b->map[bit/BITS] &= ~(1u << (bit & MASK));
}

void bitmap_toggle(bitmap_t *b, size_t bit) {
	assert(b);
	b->map[bit/BITS] ^=  (1u << (bit & MASK));
}

bool bitmap_get(bitmap_t *b, size_t bit) {
	assert(b);
	return !!(b->map[bit/BITS] & (1u << (bit & MASK)));
}

typedef int (*bitmap_foreach_callback_t)(bitmap_t *b, size_t bit, void *param);


int bitmap_foreach(bitmap_t *b, bitmap_foreach_callback_t cb, void *param) {
	const size_t bits = b->bits;
	int r = 0;
	for(size_t i = 0; i < bits; i++)
		if((r = cb(b, i, param)) < 0)
			return r;
	return 0;
}

#ifndef MIN
#define MIN(X, Y) ((X) > (Y) ? (Y) : (X))
#endif

bitmap_t *bitmap_union(bitmap_t *a, bitmap_t *b)
{
	const size_t al = bitmap_bits(a), bl = bitmap_bits(b);
	bitmap_t *u = (al > bl) ? bitmap_copy(a) : bitmap_copy(b);
	if(!u)
		return NULL;
	bitmap_t *m = (al > bl) ? b : a;
	const size_t ml = MIN(al, bl);
	for(size_t i = 0; i < ml; i++)
		if(bitmap_get(m, i))
			bitmap_set(u, i);
	return u;
}

void bitmap_print_range(bitmap_t *b, FILE *out)
{
	assert(b);
	assert(out);
	size_t total = 0;
	for(size_t i = 0; i < bitmap_bits(b);) {
		size_t start = i, end = i, j = i;
		if(!bitmap_get(b, i)) {
			i++;
			continue;
		}

		for(j = i; j < bitmap_bits(b); j++)
			if(!bitmap_get(b, j))
				break;
		end = bitmap_get(b, j) ? j :
			j > (i+1)      ? j-1 : i;
		if(start == end) {
			total++;
			fprintf(out, "    1\t%zu\n", end);
		} else {
			assert(end >= start);
			const size_t range = (end - start) + 1;
			total += range;
			fprintf(out, "%5zu\t%zu-%zu\n", range, start, end);
		}
		i = j;
	}
	fprintf(out, "total: %zu\n", total);
}

/*static int bitmap_print(bitmap_t *b, size_t bit, void *param)
{
	FILE *out = (FILE*)param;
	if(bitmap_get(b, bit))
		return fprintf(out, "%zu\n", bit) > 0;
	return 0;
}*/

int main(void) {
	bitmap_t *b1 = bitmap_new(100), *b2 = bitmap_new(1);

	fprintf(stdout, "sizeof(bitmap_unit_t) = %zu\n", sizeof(bitmap_unit_t));

	for(size_t i = 0; i < bitmap_bits(b1); i++) {
		if(!(i % 3) || !(i % 5))
			bitmap_set(b1, i);
	}

	bitmap_set(b2, 0);
	bitmap_t *u = bitmap_union(b1, b2);

	for(size_t i = 0; i < bitmap_bits(u); i ++) {
		if(bitmap_get(u, i))
			fprintf(stdout, "%zu\n", i);
	}

	bitmap_free(b1);
	bitmap_free(b2);
	bitmap_free(u);
	return 0;
}

