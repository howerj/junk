(* Then, here's toyapl_lexer.mll: *)
{
open Toyapl_parser ;;
open Toyapl_expr ;;
}
let alus = ['A'-'Z' 'a'-'z' '_']
let alnumus = alus | ['0'-'9']
let ident = alus alnumus*
rule next = parse
[' ' '\t'] { next lexbuf } (* skip whitespace; stolen from manual *)
| ['0'-'9']+ ('.' ['0'-'9']+)?
{ Num (float_of_string (Lexing.lexeme lexbuf)) }
| ['+' '-' '/' '*' '%']+ | ident
{ Op (Lexing.lexeme lexbuf) }
| '(' { Lparen }
| ')' { Rparen }
| '\n' { Eol }
