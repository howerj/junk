#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "color.h"

int main(void){
  unsigned int i,j;
  char *string = "Hello, World";
  char *p,*k;
  for(i=0;i<=c_white;i++){
    p = foreground_colorize(i,string,strlen(string));
    for(j=0;j<=c_white;j++){
      k = background_colorize(j,p,strlen(p));
      printf("%s\n",k);
      free(k);
    }
    free(p);
  }
  return 0;
}
