#include <stdlib.h>
#include <string.h>
#include "color.h"
char *foreground_colorize(enum color c, char *s, int len){
  char *ns;
  ns=calloc(len+DEFAULT_LEN+COLOR_LEN,sizeof(char));
  if(ns==NULL)
    return NULL;
  ns[0]='\033';
  ns[1]='[';
  ns[2]='3';
  ns[3]="0123456789"[c];
  ns[4]='m';
  strcpy(ns+COLOR_LEN,s);
  strcpy(ns+COLOR_LEN+len,DEFAULT_COLOR);
  return ns;
}
char *background_colorize(enum color c, char *s, int len){
  char *ns;
  ns=calloc(len+DEFAULT_LEN+COLOR_LEN,sizeof(char));
  if(ns==NULL)
    return NULL;
  ns[0]='\033';
  ns[1]='[';
  ns[2]='4';
  ns[3]="0123456789"[c];
  ns[4]='m';
  strcpy(ns+COLOR_LEN,s);
  strcpy(ns+COLOR_LEN+len,DEFAULT_COLOR);
  return ns;

  return NULL;
}
