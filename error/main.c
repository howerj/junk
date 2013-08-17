#include <stdio.h>
#include "error.c"

int main(void){
  handle_error(NULL,__LINE__,__FILE__,0);

  return 0;
}
