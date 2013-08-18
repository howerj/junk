#include <stdio.h>
#include "error.h"

int main(void){
  handle_error(NULL,__LINE__,__FILE__,0);
  return 0;
}
