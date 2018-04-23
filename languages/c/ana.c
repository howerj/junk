#include <stdio.h>
#include <string.h>

char *usage = "./ana str str";

#define POSSIBLE_STATES (256u)

int freq1[POSSIBLE_STATES], freq2[POSSIBLE_STATES];

int main(int argc,unsigned char *argv[]){
  int slen1, slen2, i;

  if(argc != 3){
    fprintf(stderr,"%s\n",usage);
    return -1;
  }

  slen1 = strlen(argv[1]);
  slen2 = strlen(argv[2]);

  if(slen1 != slen2){
    printf("Not anagrams; strlen\n");
    return 1;
  }

  memset(freq1,'\0',sizeof(int) * POSSIBLE_STATES);
  memset(freq2,'\0',sizeof(int) * POSSIBLE_STATES);

  for(i = 0; i < slen1; i++){
    freq1[*(argv[1]+i)]++;
    freq2[*(argv[2]+i)]++;
  }

  for(i = 0 ; i < POSSIBLE_STATES; i++){
    if(freq1[i] != freq2[i]){
      printf("Not anagrams\n");
      return 1;
    }
  }

  printf("Anagrams\n");
  return 0;
}
