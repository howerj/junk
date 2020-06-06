/**@brief Q-Number library
 * @copyright Richard James Howe (2018)
 * @license MIT
 *
 *  TODO:
 * 	- implement basic functions (+, -, /, %, >, <, ...)
 * 	- basic mathematical functions (sin, cos, log, pow, ...)
 * 	- printing/string operations
 * 	- different behaviors (saturation, modulo, exception)
 * 		- select behaviors by build system
 * 	- unit tests
 * NOTES:
 * 	- <https://en.wikipedia.org/wiki/Q_%28number_format%29>
 * 	- <https://www.mathworks.com/help/fixedpoint/examples/calculate-fixed-point-sine-and-cosine.html>
 * 	- <http://www.coranac.com/2009/07/sines/>
 * 	- <https://en.wikipedia.org/wiki/CORDIC>
 */

#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef BUILD_BUG_ON
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
#endif

typedef uint16_t hu_t;
typedef  int16_t hd_t;
typedef uint32_t  u_t;
typedef  int32_t  d_t;
typedef uint64_t lu_t;
typedef  int64_t ld_t;

#define BITS  (16)
#define MASK  ((1ULL << BITS) - 1ULL)
#define HIGH  (1ULL << (BITS - 1ULL))

#define UMAX  (UINT32_MAX)
#define DMIN  (INT32_MIN)
#define DMAX  (INT32_MAX)
#define LUMAX (UINT64_MAX)
#define LDMIN (INT64_MIN)
#define LDMAX (INT64_MAX)

typedef struct {
	union { u_t u; d_t d; };
} q_t;

const q_t qbit  = { .u =               1 };
const q_t qzero = { .u =       0 << BITS };
const q_t qone  = { .u =       1 << BITS };
const q_t qpi   = { .u = 0x3243F /* 3.243F6 A8885 A308D 31319 8A2E0... */};
const q_t qmin  = { .u = (HIGH << BITS) };
const q_t qmax  = { .u = (HIGH << BITS) - 1 }; 
const int qdp   = -1;

static inline d_t qsat(ld_t s) {
	if(s > DMAX) return DMAX;
	if(s < DMIN) return DMIN;
	return s;
}
static inline u_t qhigh(const q_t q) { return q.u >> BITS; }
static inline u_t qlow(const q_t q)  { return q.u & MASK; }
static inline q_t qcons(const u_t hi, const u_t lo) { return (q_t){ .u = (hi << BITS) | (lo & MASK) }; }

q_t qtrunc(q_t q) { return (q_t){ .d = q.d & (MASK << BITS) }; }

q_t qadd(q_t a, q_t b) {
	const ld_t dd = (ld_t)a.d + (ld_t)b.d;
	return (q_t){ .d = qsat(dd) };
}

q_t qsub(q_t a, q_t b) {
	const ld_t dd = (ld_t)a.d - (ld_t)b.d;
	return (q_t){ .d = qsat(dd) };
}

int qnegative(q_t a) {
	return !!(qhigh(a) & HIGH);
}

int qless(q_t a, q_t b) {
	return a.d < b.d;
}

int qmore(q_t a, q_t b) {
	return a.d > b.d;
}

int qequal(q_t a, q_t b) {
	return a.d == b.d;
}

q_t qmul(q_t a, q_t b)
{
	const ld_t dd = ((ld_t)a.d * (ld_t)b.d) + (lu_t)HIGH;
	/* NB. portable version of "dd >> BITS" */
	const lu_t ud = dd < 0 ? (-1ULL << (2*BITS)) | ((lu_t)dd >> BITS) : ((lu_t)dd) >> BITS;
	return (q_t){ .d = qsat(ud) };
}

q_t qdiv(q_t a, q_t b)
{
	ld_t dd = ((ld_t)a.d) << BITS;
	d_t bd2 = b.d / 2;
	if((dd >= 0 && b.d >= 0) || (dd < 0 && b.d < 0)) {
		dd += bd2;
	} else {
		dd -= bd2;
	}
	return (q_t) { .d = dd / b.d };
}


/*q_t qfloor(q_t q)
{
}

q_t qceil(q_t q)
{
}*/

q_t qint(int toq)  { return (q_t){ .u = ((u_t)((d_t)toq)) << BITS }; }
int qtoi(q_t toi)  { return toi.u >> BITS; }
q_t qnegate(q_t a) { return (q_t){ .u = (~a.u) + 1ULL }; }

q_t qround(q_t q)
{
	const int negative = qnegative(q);
	const int round    = !!( qlow(q) & HIGH);
	q_t r = { .d =  qsat(((ld_t)q.d) + (negative ? -round : round)) };
	return r;
}

/* <https://codereview.stackexchange.com/questions/109212> */
int qsprint(q_t p, char *s, size_t length) {
	assert(s);
	const int negative = qnegative(p);
	if(negative)
		p = qnegate(p);
	const u_t base = 10;
	const d_t hi = qhigh(p);
	char frac[BITS+1] = { 0 };
	u_t lo = qlow(p);
	for(size_t i = 0; lo; i++) {
		if(qdp >= 0 && (int)i >= qdp) /**@todo proper rounding*/
			break;
		lo *= base;
		frac[i] = '0' + (lo >> BITS);
		lo &= MASK;
	}
	return snprintf(s, length, "%s%ld.%s", negative ? "-" : "", (long)hi, frac);
}

int qprint(FILE *out, q_t p)
{
	assert(out);
	char buf[64+1] = { 0 };
	const int r = qsprint(p, buf, sizeof buf);
	return r < 0 ? r : fprintf(out, "%s", buf);
}

int qnconv(q_t *q, char *s, size_t length)
{
	assert(q);
	assert(s);
	int r = 0;
	long ld = 0;
	char frac[BITS+1] = { 0 };
	memset(q, 0, sizeof *q);
	(void)length; /* snscanf? */
	if(sscanf(s, "%ld.%16s", &ld, frac) != 2)
		return -1;
	d_t hi = ld;
	u_t lo = 0;
	if(ld > DMAX || ld < DMAX)
		r = -ERANGE;
	for(size_t i = 0; frac[i]; i++) {
		const int ch = frac[i] - '0';
		if(ch < 0)
			return -1;
		/**@todo implement this */
	}
	*q = qcons(hi, lo);
	return r;
}

int qconv(q_t *q, char *s)
{
	assert(s);
	return qnconv(q, s, strlen(s));
}

#ifdef TEST

typedef q_t (*mono)(q_t a);
typedef q_t (*duo)(q_t a, q_t b);
typedef int (*comp)(q_t a, q_t b);

static void test_mono(FILE *out, mono f, q_t a, char *op)
{
	fprintf(out, "%s(", op);
	qprint(out, a); 
	fprintf(out, ") = ");
	qprint(out, f(a)); 
	fputc('\n', out);
}

static void test_duo(FILE *out, duo f, q_t a, q_t b, char *op)
{
	qprint(out, a); 
	fprintf(out, " %s ", op);
	qprint(out, b); 
	fprintf(out, " = ");
	qprint(out, f(a, b)); 
	fputc('\n', out);
}

static void test_comp(FILE *out, comp f, q_t a, q_t b, char *op)
{
	qprint(out, a); 
	fprintf(out, " %s ", op);
	qprint(out, b); 
	fprintf(out, " = %s", f(a, b) ? "true" : "false");
	fputc('\n', out);
}

static void printq(FILE *out, q_t q, const char *msg)
{
	fprintf(out, "%s = ", msg);
	qprint(out, q);
	fputc('\n', out);
}

int main(void) {
	/**@todo more static assertions relating to min/max numbers, BITS and
	 * MASK */
	BUILD_BUG_ON( sizeof(u_t) !=  sizeof(d_t));
	BUILD_BUG_ON(sizeof(lu_t) !=  sizeof(ld_t));
	BUILD_BUG_ON(sizeof(hu_t) !=  sizeof(hd_t));
	BUILD_BUG_ON(sizeof(u_t)  != (sizeof(hu_t) * 2));
	BUILD_BUG_ON(sizeof(lu_t) != (sizeof(u_t)  * 2));

	FILE *out = stdout;
	q_t p1 = qcons(3, HIGH/5);
       	q_t p2 = qnegate(p1);
	q_t p3 = qint(2);
	test_duo(out, qadd, p1, p2, "+");
	test_duo(out, qadd, p1, p1, "+");
	test_duo(out, qsub, p1, p2, "-");
	test_duo(out, qsub, p1, p1, "-");
	test_duo(out, qmul, p1, p2, "*");
	test_duo(out, qmul, p1, p1, "*");
	test_duo(out, qdiv, p1, p2, "/");
	test_duo(out, qdiv, p1, p1, "/");
	test_duo(out, qdiv, p1, p3, "/");
	test_duo(out, qdiv, p2, p3, "/");

	test_mono(out, qnegate, qpi, "negate");
	test_mono(out, qnegate, p2, "negate");
	test_mono(out, qtrunc,  p1, "trunc");
	test_mono(out, qtrunc,  p2, "trunc");


	test_comp(out, qequal, p1, p1, "==");
	test_comp(out, qequal, p1, p2, "==");


	test_comp(out, qmore, p1, p2, ">");
	test_comp(out, qmore, p2, p1, ">");
	test_comp(out, qmore, p1, p1, ">");

	test_comp(out, qless, p1, p2, "<");
	test_comp(out, qless, p2, p1, "<");
	test_comp(out, qless, p1, p1, "<");

	printq(out, qbit,  "bit");
	printq(out, qone,  "one");
	printq(out, qzero, "zero");
	printq(out, qpi,   "pi");
	printq(out, qmin,  "min");
	printq(out, qmax,  "max");

	q_t conv = qzero;
	qconv(&conv, "-12.34"); /**@todo fix this */
	qprint(out, conv);
	fputc('\n', out);


	return 0;
}
#endif

