#include <ncurses.h>
#include <string.h>

/* From:
 * https://codegolf.stackexchange.com/questions/375/build-an-engine-for-a-maze-game?rq=1
 * 
 * gcc maze.c -lncurses -o maze
 * ./maze maze.txt
 * 
$ cat maze.txt
+-+-+
  |#|
|   |
+---+
*/


#define MIN(A,B) (A<B?A:B)/*unsafe,but short*/
#define MAX(A,B) (A>B?A:B)/*unsafe,but short*/
// #define MAX(A,B) ((_A=A)>(_B=B)?_A:_B) /* safe but verbose */
#define T(C,X,Y) case C:if((m[x+X][y+Y]==' ')||(m[x+X][y+Y]=='#'))x+=X,y+=Y;s++;break;
char m[24][81],M[24][81];/* [m]ap and [M]ask; NB:mask intialized by default */
int i,j, /* loop indicies over the map */
  I=0,J, /* limits of the map */
  x,y,   /* player position */
  s=0;   /* steps taken */
int main(int c,char**v){
  FILE*f=fopen(v[1],"r"); /* fragile, assumes that the argument is present */
  /* Read the input file */
  for(I=0;fgets(m[I],80,f);I++)J=MAX(J,strlen(m[I])); /* Read in the map */ 
  J--;
  /* note that I leak a file handle here */
  f=fopen("/dev/random","r");
  /* Find a open starting square */
  do{ 
    x=fgetc(f)%I; /* Poor numeric properties, but good enough for code golf */
    y=fgetc(f)%J;
  } while(m[x][y]!=' ');
  /* setup curses */
  initscr(); /* start curses */
  //  raw();     /* WARNING! intercepts C-c, C-s, C-z, etc...
  //          * but shorter than cbreak() 
  //          */
  curs_set(0); /* make the cursor invisible */
  /* main loop */
  do {
    switch(c){
      T('e',0,1)
      T('n',-1,0)
      T('s',1,0)
      T('w',0,-1)
    }
    /* Update the mask */
    for(i=MAX(0,x-1);i<MIN(x+2,I);i++)
      for(j=MAX(0,y-1);j<MIN(y+2,J);j++)
    M[i][j]=1;
    /* draw the maze as masked */
    for(i=0;i<I;i++)
      for(j=0;j<J;j++)
    mvaddch(i,j,M[i][j]?m[i][j]:'?');
    /* draw the player figure */
    mvaddch(x,y,'@');
    refresh(); /* Refresh the display */
  } while((m[x][y]!='#')&&(c=getch())!='q');
  if(m[x][y]=='#')mvprintw(I,0,"Finished in %d steps!",s),getch();
  endwin();
}
