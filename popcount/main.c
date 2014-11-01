/*simple _unsafe_ test bench for popcount*/

#include <stdio.h>
#include <string.h>
#include "popcount.h"

int main(int argc, char **argv){
	if(argc>1)
		while(argc-->1)
			printf("%s %d\n", argv[argc], popcount(atoi(argv[argc])));
	else
		return fprintf(stderr, "usage: %s num*\n", argv[0]);
	return 0;
}
