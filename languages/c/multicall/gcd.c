#include "app.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int gcd(int a,int b)
{
	int c;
	while(a){
		c = a; 
		a = b%a;
		b = c;
	}
	return b;
}

int main_gcd(int argc, char **argv){
	if(argc>2)
		printf("%d\n",gcd(atoi(argv[1]),atoi(argv[2])));
	else
		return fprintf(stderr,"usage: %s num num\n",argv[0]);
	return 0;
}

