/*Richard Howe
 * Simple grep test program
 */
#include <stdio.h>
#include "regex.h"

#define LINEBUF_LEN 256

int main(int argc, char *argv[]){
  FILE *input;
  char linebuf[LINEBUF_LEN];
  int i,j;

  if(argc>2){ /*read files*/
    for(i=2;i<argc;i++){
      if((input=fopen(argv[i],"r"))==NULL){
        fprintf(stderr,"Could not open %s for reading.\n",argv[i]);
        return 1;
      } else{
        fprintf(stderr,"%s:\n",argv[i]);
      }
      for(j=0;fgets(linebuf,LINEBUF_LEN,input)!=NULL;j++){
        if(match(argv[1],linebuf)){
          printf("line <%d>: %s\n", j, linebuf);
        }
      }
      fclose(input);
    }
  } else if (argc == 2){ /*reading from stdin*/
    fprintf(stderr,"Reading from stdin\n");
    for(j=0;fgets(linebuf,LINEBUF_LEN,stdin)!=NULL;j++){
        if(match(argv[1],linebuf)){
          printf("line <%d>: %s\n", j, linebuf);
        }
    }
  } else {
    fprintf(stderr,"Usage: %s <expression> <file #1> ... <file #N>\n", argv[0]);
    return 1;
  }

  return 0;
}
