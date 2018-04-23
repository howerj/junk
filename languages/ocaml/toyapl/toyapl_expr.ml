(* First, here's an expression type, which I put in toyapl_expr.ml *)
type value = float list ;;
type op = string ;;
type expr =
Unary of op * expr
| Atom of value
| Parenthesized of expr
| Binary of expr * op * expr ;; (* later, add variables *)
