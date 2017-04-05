/* a very simple demonstration multi call binary */
#include <stdio.h>
#include <string.h>
#include "app.h" /* grep main_ *.c | grep -v "^main" | awk 'BEGIN{FS=":"}{print $2}' | sed "s/{$/;/" */

struct util_list{
	char *name;
	int (*func)(int argc, char **argv);
	char *description;
} list[] = {
	{"multicall", NULL,          "A (very simple) multicall binary"},
	{"gcd",       main_gcd,      "Given two numbers it will find their Greatest Common Divisor (GCD)"},
	{"note",      main_note,     "A simple note taking application"},
	{"popcount",  main_popcount, "Given a list of numbers perform a population count for each one"},
	{"rm",        main_rm,       "Remove a list of file"},
	{"mv",        main_mv,       "Move a file"},
	{NULL,        NULL,          ""}
};

int main(int argc, char **argv){
	int i;
	for(i = 0; NULL != list[i].name; i++)
		if((NULL != list[i].func) && !strcmp(argv[0]+2, list[i].name)) /* +2 skip ./ */
			return list[i].func(argc,argv);

	for(i = 0; NULL != list[i].name; i++)
		printf("%s: %s\n", list[i].name, list[i].description);

	return 0;
}
