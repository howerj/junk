/*from Rosetta Code; A terminal based note taking program*/
#include "app.h"
#include <stdio.h>
#include <time.h>

#define note_file "notes.txt"

int main_note(int argc, char **argv){
	FILE *note = NULL;
	time_t tm;
	int i;
	char *p;

	if(argc < 2){
		if((note = fopen(note_file, "r")))
			while((i = fgetc(note)) != EOF)
				putchar(i);
	} else if((note = fopen(note_file, "a"))) {
		tm = time(0);
		p = ctime(&tm);

		while(*p)
			fputc(*p != '\n'?*p:'\t',note), p++;

		for(i = 1; i < argc; i++)
			fprintf(note, "%s%c", argv[i], 1 + i - argc ? ' ' : '\n');
	}

	if(note)
		fclose(note);

	return 0;
}
