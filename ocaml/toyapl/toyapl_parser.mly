%{
open Toyapl_expr;;
%}
%token <float> Num
%token <Toyapl_expr.op> Op
%token Lparen Rparen Eol
%right Op
%start parse_line
%type <Toyapl_expr.expr> parse_line
%%
parse_line:
expr Eol { $1 }
;
atom:
Num { [ $1 ] }
| atom Num { $1 @ [ $2 ] }
expr:
atom { Atom $1 }
| expr Op expr { Binary($1, $2, $3) }
| Lparen expr Rparen { Parenthesized($2) }
| Op expr { Unary($1, $2) }
| Lparen Rparen { Atom([]) }
