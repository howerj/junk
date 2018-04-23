#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SMAX 999
#define CMAX 256
#define BMAX 999
#define LMAX 9999
typedef char * STRPTR ;

int E [ SMAX ] ; /* subroutine line number stack */
int L [ CMAX ] ; /* FOR loop beginning line number */
int M [ CMAX ] ; /* FOR loop maximum index value */
int P [ CMAX ] ; /* program variable value */
int l , i , j ;
int * C ; /* subroutine stack pointer */
char B [ BMAX ] ; /* command input buffer */
char F [ 2 ] ; /* temporary search string */
STRPTR m [ 1 + LMAX ] ; /* pointers to lines of program */
STRPTR p , q , x , y , z , s , d ;
FILE * f ; /* file pointer for input and output */

STRPTR dds_strstr ( STRPTR s , STRPTR o )
{
STRPTR x , y , z ;
 for ( x = s ; * x ; x ++ ) {
  for ( y = x , z = o ; * z && * y == * z ; y ++ ) z ++ ;
  if ( z > o && ! * z ) return x ;
 } return 0 ;
} /* end dds_strstr */

void G ( void ) ;
int S ( void ) ;
int J ( void ) ;
int K ( void ) ;
int V ( void ) ;
int W ( void ) ;
int Y ( void ) ;

int main ( void ) {
printf("%c%c",0x1b,'c');
puts("");
puts("** DDS-BASIC interpreter annotated by Michael Somos 1997");
puts("** original by Diomidis Spinellis for 1990 IOCCC");
puts("** compiled for muLinux, with minor changes.");
puts("");
puts("Immediate commands:");

puts("\tRUN     LIST    NEW     OLD filename\n\
\tBYE     SAVE filename\
");
puts("Note:\n\tALL INPUT MUST BE UPPERCASE!");
puts("");

m [ LMAX ] = "E" ; /* "END" */
while ( puts ( "Ok" ) , fgets ( B , BMAX , stdin ) )
 switch ( * B ) {
 case 'R' : /* "RUN" command */
  C = E ;
  l = 1 ;
  for ( i = 0 ; i < CMAX ; P [ i ++ ] = 0 ) ; /* initialize variables */
  while ( l ) {
   while ( ! ( s = m [ l ] ) ) l ++ ;
   if ( ! dds_strstr ( s , "\"" ) ) {
     while ( ( p = dds_strstr ( s , "<>" ) ) ) * p ++ = '#' , * p = ' ' ;
     while ( ( p = dds_strstr ( s , "<=" ) ) ) * p ++ = '$' , * p = ' ' ;
     while ( ( p = dds_strstr ( s , ">=" ) ) ) * p ++ = '!' , * p = ' ' ;
   }
   d = B ;
   while ( ( * F = * s ) ) {
     if ( * s == '"' ) j ++ ;
     if ( j & 1 || ! dds_strstr ( " \t" , F ) )
       * d ++ = * s ;
     s ++ ;
   }
   * d -- = j = 0 ;
   if ( B [ 1 ] != '=' ) switch ( * B ) {
     case 'E' : /* "END" */
       l = - 1 ;
       break ;
     case 'R' : /* "REM" */
       if ( B [ 2 ] != 'M' ) l = * -- C ; /* "RETURN" */
       break ;
     case 'I' :
       if ( B [ 1 ] == 'N' ) { /* "INPUT" */
       fgets ( p = B , BMAX , stdin ) ; P [ * d ] = S ( ) ;
       } else { /* "IF" */
       * ( q = dds_strstr ( B , "TH" ) ) = 0 ; /* "THEN" */
       p = B + 2 ;
       if ( S ( ) ) { p = q + 4 ; l = S ( ) - 1 ; }
       }
       break ;
     case 'P' : /* "PRINT" */
       if ( B [ 5 ] == '"' ) {
         * d = 0 ; puts ( B + 6 ) ;
       } else {
         p = B + 5 ;
         printf ( "%d\n" , S ( ) ) ;
       }
       break ;
     case 'G' : /* "GOTO" */
       p = B + 4 ;
       if ( B [ 2 ] == 'S' ) { /* "GOSUB" */
         * C ++ = l ; p ++ ; }
       l = S ( ) - 1 ;
       break ;
     case 'F' : /* "FOR" */
       * ( q = dds_strstr ( B , "TO" ) ) = 0 ; /* "TO" */
       p = B + 5 ;
       P [ i = B [ 3 ] ] = S ( ) ;
       p = q + 2 ;
       M [ i ] = S ( ) ;
       L [ i ] = l ;
       break ;
     case 'N' : /* "NEXT" */
        if ( ++ P [ * d ] <= M [ * d ] ) l = L [ * d ] ;
     } else {
     p = B + 2 ;
     P [ * B ] = S ( ) ; }
   l ++ ;
  } /* end while l */
    break ;
  case 'L' : /* "LIST" command */
    for ( i = 0 ; i < LMAX ; i ++ )
    if ( m [ i ] ) printf ( "%d %s\n", i , m [ i ] ) ;
    break ;
  case 'N' : /* "NEW" command */
    for ( i = 0 ; i < LMAX ; i ++ )
    if ( m [ i ] ) { free ( m [ i ] ) ; m [ i ] = 0 ; }
    break ;
  case 'B' : /* "BYE" command */
    return 0 ;
    break ;
  case 'S' : /* "SAVE" command */
    f = fopen ( B + 5 , "w" ) ;
    for ( i = 0 ; i < LMAX ; i ++ )
      if ( m [ i ] ) fprintf ( f , "%d %s\n" , i , m [ i ] ) ;
    fclose ( f )  ;
    break ;
  case 'O' : /* "OLD" command */
    f = fopen ( B + 4 , "r" ) ;
    while ( fgets ( B , BMAX , f ) ) {
      * dds_strstr ( B , "\n" ) = 0 ;
      G ( ) ;
    }
    fclose ( f )  ;
    break ;
  case 0 :
  default :
    G ( ) ;
} /* end switch *B */
return 0 ;
} /* end main */

void G ( void ) { /* get program line from buffer */
l = atoi ( B ) ;
if ( m [ l ] ) free ( m [ l ] ) ;
if ( ( p = dds_strstr ( B , " " ) ) )
  strcpy ( m [ l ] = malloc ( strlen ( p ) ) , p + 1 ) ;
  else m [ l ] = 0 ;
} /* end G */

/* recursive descent parser for arithmetic/logical expressions */
int S ( void ) {
int o = J ( ) ;
switch ( * p ++ ) {
  case '=' : return o == S ( ) ;
    break ;
  case '#' : return o != S ( ) ;
  default : p -- ; return o ;
  }
} /* end S */

int J ( void ) {
int o = K ( ) ;
switch ( * p ++ ) {
  case '<' : return o < J ( ) ;
    break ;
  case '>' : return o > J ( ) ;
  default : p -- ; return o ;
  }
} /* end J */

int K ( void ) {
int o = V ( ) ;
switch ( * p ++ ) {
  case '$'  : return o <= K ( ) ;
    break ;
  case '!'  : return o >= K ( ) ;
  default : p -- ; return o ;
  }
} /* end K */

int V ( void ) {
int o = W ( ) ;
switch ( * p ++ ) {
  case '+'  : return o + V ( ) ;
    break ;
  case '-'  : return o - V ( ) ;
  default : p -- ; return o ;
  }
} /* end V */

int W ( void ) {
int o = Y ( ) ;
switch ( * p ++ ) {
  case '*'  : return o * W ( ) ;
    break ;
  case '/'  : return o / W ( ) ;
  default : p -- ; return o ;
  }
} /* end W */

int Y ( void ) {
int o ;
return
( * p == '-' ) ? ( p ++ , - Y ( ) ) :
  ( * p >= '0' && * p <= '9' ) ? strtol ( p , & p , 0 ) :
    ( * p == '(' ) ?
      ( p ++ , o = S ( ) , p ++ , o ) :
      P [ * p ++ ] ;
} /* end Y */

/* end of DDS-BASIC program */
