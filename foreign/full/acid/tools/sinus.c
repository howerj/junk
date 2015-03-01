#include <math.h>
#include <stdio.h>

#define LOOPS  16

int
main ()
{
	int i;
	double k = 6.28/LOOPS;

	for (i = 0; i < LOOPS; i++)
		printf ("\t\tdb\t%d\n", (int)floor ((sin (k*i) + 1) * 3 + 0.5));
}
