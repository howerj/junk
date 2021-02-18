#include <stdio.h>
int main(int x,char**v){FILE*F=fopen(v[1],"r");short p=0,m[65536],*i=m;
while(fscanf(F,"%hd",i++)>0);while(p>=0){int a=m[p++],b=m[p++],c=m[p++];
a<0?m[b]=getchar():b<0?putchar(m[a]):(m[b]-=m[a])<=0?p=c:0;}}
