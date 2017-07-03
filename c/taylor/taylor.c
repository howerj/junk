#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

static uint64_t factorial(uint64_t x)
{
	uint64_t y = x;
	if(!x)
		return 1;
	for(uint64_t i = 1; i < x; i++)
		y *= i;
	return y;
}

static double sine(double x, unsigned max)
{
	double r = 0;
	for(unsigned i = 0; i < max; i++) {
		unsigned m = i * 2 + 1;
		double c = pow(x, m) / (double)factorial(m);
		r = i & 1 ? r - c : r + c;
	}
	return r;
}

int main(void)
{
	double x = 0., r = 0., a, adiff;
	unsigned max = 0;

	printf("sine of: ");
	scanf("%lf", &x);

	printf("max iterations: ");
	scanf("%u", &max);
	r = sine(x, max);

	printf("x:                 %f\n", x);
	printf("iterations:        %u\n", max);
	printf("calculated sin(x): %f\n", r);
	printf("math.h sin(x):     %f\n", a = sin(x));
	printf("difference:        %f\n", adiff = fabs(a-r));

	return 0;
}

