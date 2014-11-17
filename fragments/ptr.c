#include <stdio.h>

typedef struct{
  char c;
  int i;
} test_t;

test_t t[10] = {
  {'h','1'},
  {'e','2'},
  {'l','3'},
  {'l','3'},
  {'o','4'},
 
  {'w','5'},
  {'o','4'},
  {'r','6'},
  {'l','3'},
  {'d','7'},
} ;

test_t (*tp)[] = &t;

int main(void){
  int i = 0;
  for(i = 0; i< 10; i++){
    printf("%d: %c\n",i, (*tp)[i].c);
  }

  return 0;
}
