#include "regex.h"
#include <stdio.h>
#include <stdlib.h>

void usage(const char *arg0)
{
	fprintf(stderr, "usage: %s regex string\n", arg0);
}

int main(int argc, char **argv)
{
	if(argc != 3) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	fprintf(stdout, "regex:\t%s\nstring:\t%s\n%s\n", argv[1], argv[2], match(argv[1], argv[2]) ? "match" : "no match");
	return 0;
}
