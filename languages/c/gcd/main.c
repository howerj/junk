/*gcd of args, unsafe*/
#include <stdio.h>
#include <string.h>
#include "gcd.h"

int main(int argc, char **argv){
	if(argc>2)
		printf("%d\n",gcd(atoi(argv[1]),atoi(argv[2])));
	else
		return fprintf(stderr,"usage: %s num num\n",argv[0]);
	return 0;
}
