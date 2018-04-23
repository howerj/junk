/* From https://stackoverflow.com/questions/811074/what-is-the-coolest-thing-you-can-do-in-10-lines-of-simple-code-help-me-inspir?page=2&tab=votes#tab-top */
#include <stdlib.h>  
#include <stdio.h>  

long a=10000,b,c=2800,d,e,f[2801],g;  

int main()  
{  
    for(;b-c;)  
        f[b++]=a/5;  
    for(;d=0,g=c*2;c-=14,printf("%.4d",e+d/a),e=d%a)  
        for(b=c;d+=f[b]*a,f[b]=d%--g,d/=g--,--b;d*=b);  
}  
