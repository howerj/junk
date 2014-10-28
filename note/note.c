#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUF_SZ (512u)

char *notefilename = "notes.txt";
char *usage = "./note <optional string>\n";

FILE *safefopen(char *name, char *mode){
  FILE *f;
  f = fopen(name,mode);
  if(NULL == f){
    fputs("file open failed\n",stderr);
    exit(-1);
  }
  return f;
}

int main(int argc, char *argv[]){
  FILE *note;
  if(argc > 2){ /* too many arguments */
    fputs(usage,stderr);
    return EXIT_FAILURE;
  } else if(argc == 2){ /* note in the form of "note" */
    time_t time_cur;
    char *time_str;
    time_cur = time(NULL);
    time_str = ctime(&time_cur);
    note = safefopen(notefilename,"a");
    fwrite(time_str, sizeof(char), strlen(time_str) - 1, note);
    fprintf(note," %s\n",argv[1]);
    fclose(note);
  } else { /* read note file */
    char buf[BUF_SZ];
    int count;
    note = safefopen(notefilename,"r");
    do{
      memset(buf,'\0',BUF_SZ);
      count = fread(buf, sizeof(char), BUF_SZ, note);
      puts(buf);
    } while(BUF_SZ == count);
    fclose(note);
  }
  return EXIT_SUCCESS;
}
