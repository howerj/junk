#include "app.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char *separator = " --> ";

int main_mv(int argc, char **argv){
        if(argc==3){
                if(rename(argv[1],argv[2]) < 0){
                        size_t s1_len = strlen(argv[1]);
                        size_t sep_len = strlen(separator);
                        size_t sz = s1_len + sep_len + strlen(argv[2]) + 1;
                        char *s = malloc(sz);
                        strcat(s,argv[1]);
                        strcat(s+s1_len,separator);
                        strcat(s+s1_len+sep_len,argv[2]);
                        perror(s);
                        free(s);
                }
        } else {
		return fprintf(stderr,"usage: %s source destination\n",argv[0]);
        }
	return 0;
}

