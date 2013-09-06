#include <stdio.h>
#include <stdlib.h>

double my_log(double log_me)
{
  double x,i;
  for(x = log_me; x > 1; x = x - (x/100.0f),i++);
  i+=x;
  return i/229.15f;
}
int main(void) {
  printf("100 %f\n",my_log(100));
  printf("50 %f\n",my_log(50));
  printf("30 %f\n",my_log(30));
  printf("10 %f\n",my_log(10));
  printf("5 %f\n",my_log(5));
  printf("3 %f\n",my_log(3));
  return EXIT_SUCCESS;
}
