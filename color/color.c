#include <stdlib.h>
#include "color.h"
char *foreground_colorize(enum color c, char *s, int len){
  char *ns;
  ns=calloc(len+DEFAULT_LEN+COLOR_LEN,sizeof(char));
  if(ns==NULL)
    return NULL;
  

  return ns;
}
char *background_colorize(enum color c, char *s, int len){
  return NULL;
}
