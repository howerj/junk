#include "app.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main_rm(int argc, char **argv){
        int i;
        if(argc>1){
                for(i = 1; i < argc; i++)
                        if(remove(argv[i]) < 0)
                                perror(argv[i]);
        } else {
		return fprintf(stderr,"usage: %s file*\n",argv[0]);
        }
	return 0;
}

