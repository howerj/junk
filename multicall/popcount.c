#include "app.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static unsigned int popcount(unsigned int c){
	unsigned int x = 0;
	do x += c & 1; while(c >>= 1);
	return x;
}

int main_popcount(int argc, char **argv){
	if(argc>1)
		while(argc-->1)
			printf("%s %d\n", argv[argc], popcount(atoi(argv[argc])));
	else
		return fprintf(stderr, "usage: %s num*\n", argv[0]);
	return 0;
}
