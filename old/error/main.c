#include <stdio.h>
#include "error.h"

int main(void){
  int i;
  error_t e = {NULL,NULL,0,0,error_ok,false,false};
  e.stderr_output = stderr;

  handle_error(NULL,__LINE__,__FILE__,0);

  for(i = 0; i < error_LAST_ERROR; i++)
    handle_error(&e,__LINE__,__FILE__,i);

  return 0;
}
