#include "popcount.h"
unsigned int popcount(unsigned int c){
	unsigned int x = 0;
	do x += c & 1; while(c >>= 1);
	return x;
}
