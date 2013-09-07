#include <stdio.h>
#include <stdlib.h>

double my_log(double log_me)
{
  double x,i;
  for(x = log_me; x >= 1; x = x - (x/100.0f),i++); /*(x = x - (x/100)) == X*0.99*/
  i+=x; /*add remainder*/
  return i/229.15f; /*229.15 = -1/log(.99) */
}

double my_antilog(double antilog_me){
  double x=10,i,j; 
  i = (1-antilog_me) * 229.15f;
  if(i<0)
    i*=-1;
  for(; i > 0; i--, x = x  - (x/100.0f));
  return 10/(x);
}

int main(void) {
  printf("100 %f\n",my_log(100));
  printf("50 %f\n",my_log(50));
  printf("30 %f\n",my_log(30));
  printf("10 %f\n",my_log(10));
  printf("5 %f\n",my_log(5));
  printf("3 %f\n",my_log(3));
  printf("==================\n");
  printf("1 %f\n",my_antilog(1));
  printf("2 %f\n",my_antilog(2));
  printf("3 %f\n",my_antilog(3));
  printf("4 %f\n",my_antilog(4));
  printf("5 %f\n",my_antilog(5));
  return EXIT_SUCCESS;
}
