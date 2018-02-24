/* Tiny Parser Combinator Example
 * https://groups.google.com/forum/#!msg/comp.lang.c/BuwBXfpgN_M/4I4krY2PAgAJ
 * https://www.reddit.com/r/tinycode/comments/6hogqb/parser_combinators_in_100_lines_of_c/
 */

#include <stdio.h>
#include <stdlib.h>

typedef struct parser *parser;
typedef struct result *result;
typedef result (*handler)( parser p, char *input );

struct parser {
  handler test;
  parser a, b;
  int c;
};

struct result {
  result next;
  int length_matched;
};

#define new_function_for_(type) \
  type new_##type ( struct type input ){ \
    type local = calloc( sizeof *local, 1 ); \
    return  local  ?  ( *local = input ), local  :  local; \
  }
new_function_for_(parser)
new_function_for_(result)


result parse( parser p, char *input ){  return  p->test( p, input );  }
result parse_fail( parser p, char *input ){  return  NULL;  }
result parse_succeed( parser p, char *input ){
  return  new_result( (struct result){ .next = NULL, .length_matched = 0 } );
}
result parse_term( parser p, char *input ){
  return  *input == p->c  ?
    new_result( (struct result){ .next = NULL, .length_matched = 1 } ) :
    NULL;
}

result parse_alternate( parser p, char *input ){
  result res = parse( p->a, input );
  if(  res  ){
    result r = res;
    while(  r->next  ) r = r->next;
    r->next = parse( p->b, input );
    return  res;
  } else  return  parse( p->b, input );
}

void amend_lengths( result r, int length ){
  if(  r  )  r->length_matched += length;
  if(  r->next  )  amend_lengths( r->next, length );
}

result parse_sequence_next( result r, parser p, char *input ){
  if(  ! r  )  return  NULL;
  result res = parse( p->b, input + r->length_matched );
  if(  res  ){
    amend_lengths( res, r->length_matched );
    result tail = res;
    while(  tail->next  )  tail = tail->next;
    tail->next = parse_sequence_next( r->next, p, input );
    return  res;
  } else  return  parse_sequence_next( r->next, p, input );
}

result parse_sequence( parser p, char *input ){
  result res = parse( p->a, input );
  return  parse_sequence_next( res, p, input );
}


parser fails(){  return  new_parser( (struct parser){ .test = parse_fail } ); }
parser succeeds(){  return  new_parser( (struct parser){ .test = parse_succeed } ); }

parser term( int c ){
  return  new_parser( (struct parser){ .test = parse_term, .c = c } );
}
parser alternate( parser a, parser b){
  return  new_parser( (struct parser){ .test = parse_alternate, .a = a, .b = b } );
}
parser sequence( parser a, parser b){
  return  new_parser( (struct parser){ .test = parse_sequence, .a = a, .b = b } );
}
parser many( parser a ){
  parser q = new_parser( (struct parser){ .test = parse_sequence, .a = a } );
  parser r = new_parser( (struct parser){ .test = parse_alternate, .a = q, .b = succeeds() } );
  q->b = r;
  return r;
}
parser some( parser a ){
  parser q = new_parser( (struct parser){ .test = parse_sequence, .a = a } );
  parser r = new_parser( (struct parser){ .test = parse_alternate, .a = q, .b = succeeds() } );
  q->b = r;
  return q;
}

parser char_class( char *str ){
  return  *str  ?  alternate( term( *str ), char_class( str + 1 ) )  :  fails();
}
parser string( char *str ){
  return  *str  ?  sequence( term( *str ), string( str + 1 ) )  :  succeeds();
}


void print_results( result res ){
  if(  res  )  printf( "%d ", res->length_matched ),
                 print_results( res->next );
}

void test( parser p, char *str ){
  printf( "%s:", str );
  fflush(0);
  result r = parse( p, str );
  print_results( r );
  printf( "\n" );
}

int main(){
  parser p = string("hello");
  test( p, "hello" );
  test( p, "hell" );
  test( p, "hello w" );
  test( p, "xxxxx" );

  parser q = many( term( 'x' ) );
  test( q, "xy" );
  test( q, "xx" );
  test( q, "xxxy" );
  test( q, "xxxxy" );
  test( q, "xxxxx" );
 
  parser r = sequence( many( term( 'x' ) ), string( "xy" ) );
  test( r, "xy" );
  test( r, "xx" );
  test( r, "xxxy" );
  test( r, "xxxxy" );
  test( r, "xxxxx" );
  return 0;
} 
