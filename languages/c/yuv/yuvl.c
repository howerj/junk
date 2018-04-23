/* 

This plays the Game of Life and generates a YUV MPEG2
video as its output. It was de-obfuscated from a piece
of code posted on Hacker News by user "Retr0spectrum":

	<https://news.ycombinator.com/item?id=14684003>


The original code:

    #define  F for(i=0;i<l*3;i++)
    main(n){int i,j,w=640,h=480,l
    =w*h,o[]={~w,-w,-w+1,-1,1,w-1
    ,w,w+1},b[l*5];F b[i]=rand();
    for(puts("YUV4MPEG2 W640 H48"
    "0 F30 C444");puts("FRAME");)
    {F{for(n=j=8;j;n-=b[i+o[--j]]
    &1);b[i+l]=(n^5&&!b[i]|n^6)-1
    ;}F putchar(b[i]=b[i+l]);}}

It could do with some more cleaning up, but is functional.
*/
#include <stdio.h>
#include <stdlib.h>

#define w (640)
#define h (480)
#define l (w*h)

int main(void)
{
	static int i, j, n,
	o[] = {
		~w, -w, -w+1,
		-1, 1,
		w-1, w, w+1
	},
	b[l*5];

	for(i = 0; i < l * 3; i++)
		b[i] = rand();

	puts("YUV4MPEG2 W640 H480 F30:1 C444");
	for(;;) {
		puts("FRAME");
		for(i = 0; i< l * 3; i++) {
			for(n = 8, j = 8; j; n -= b[i + o[--j]] & 1)
				/* do nothing */;
			
			b[i+l] = ((n^5) && ((!b[i]) | (n^6))) - 1;
		}
		for(i=0;i<l * 3;i++)
			putchar(b[i]=b[i+l]);
	}
	return 0;
}

