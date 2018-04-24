%{
#include "miniprolog.h"
extern FILE *yyin;
int yylineno;
extern int numProgRules;
extern int structure;
%}
%token <string_val> VARIABLE ID
%token COLON DOT NEWLINE COMMA LPARENT RPARENT MID QUERY 
%union  {
        char   *string_val;
}

%start start

%%
    start : { START_STAT(); } start_prog end_prog { END_STAT();};

    start_prog: QUERY { START_QUERY(); } q { END_QUERY(); } | 
        rule { ADD_RULE(); } | /*empty*/;

    end_prog: NEWLINE;

    q : tail DOT;

    rule: clause DOT | unit_clause DOT;

    clause: unit_clause MID tail;

    tail: unit_clause end_tail;

    end_tail: COMMA unit_clause end_tail | /* empty */;

    unit_clause: constant | functor;

    constant: ID {
       assignConstant($<string_val>1);
    }
    ;

    functor: ID{
       START_STRUCTURE(); 
       FUNCTOR($<string_val>1); 
       assignStructure($<string_val>1); 
    } functor_tail;

    functor_tail :  LPARENT term end_term RPARENT { END_STRUCTURE(); };

    term: functor | constant | VARIABLE { 
       assignVariable($<string_val>1);
    };
    
    end_term: COMMA term end_term | /* empty */;                   
%%

yyerror(char *s){  
    printf("%s @ line %d\n", s, yylineno-1);
    exit(1);    
}