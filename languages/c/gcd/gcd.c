#include "gcd.h"
int gcd(int a,int b)
{
	int c;
	while(a){
		c = a; 
		a = b%a;
		b = c;
	}
	return b;
}
