(* And here's the main program, toyapl.ml: *)
(* toy APL as an exercise to learn OCaml *)
(* to do: change values to trees *)
open Toyapl_expr ;;

exception Op_not_found of op ;;

let assoc f list = try List.assoc f list
with Not_found -> raise (Op_not_found f) ;;

let rec eval_with unaries binaries = let rec eval = function
Unary (f, e) -> (assoc f unaries) (eval e)
| Atom e -> e
| Parenthesized e -> eval e
| Binary (e1, f, e2) -> (assoc f binaries) (eval e1) (eval e2)
in eval ;;

let show_num n = if n = float_of_int (int_of_float n)
then string_of_int (int_of_float n)
else string_of_float n ;;

let rec show_atom = function
[] -> "()"
| n :: m :: lst -> show_num n ^ " " ^ show_atom (m::lst)
| [n] -> show_num n ;;

let rec show = function
Unary (f, e) -> f ^ " " ^ show e
| Atom e -> show_atom e
| Parenthesized e -> "(" ^ show e ^ ")"
| Binary (Atom _ as e1, f, e2) | Binary (Parenthesized _ as e1, f, e2) ->
show e1 ^ " " ^ f ^ " " ^ show e2
| Binary (e, f, e2) ->
show (Parenthesized e) ^ " " ^ f ^ " " ^ show e2 ;;

(* numbers up to n. *)
(* it's not clear what iota should do when applied to a nonscalar *)
let apl_iota [n] = let rec iota start stop = if start = stop then []
else float_of_int start :: iota (start + 1) stop
in iota 0 (int_of_float n) ;;

let unary_lift = List.map ;;
let reduce op id x = [List.fold_left op id x] ;;
let unaries = ["+", unary_lift (fun x -> x); "-", unary_lift (~-.);
"+/", reduce (+.) 0.;
"*/", reduce ( *. ) 1.;
"iota", apl_iota] ;;

exception Mismatched_list_lengths of value * value ;;

let aplbinarylift f a b =
let flip f a b = f b a
in
match (a, b) with
([a1], [b1]) -> [f a1 b1]
| ([a1], b1 :: b2 :: rest) -> List.map (f a1) b
| (a1 :: a2 :: rest, [b1]) -> List.map (flip f b1) a
| (_, _) -> try List.map2 f a b with
Invalid_argument _ -> raise (Mismatched_list_lengths (a, b)) ;;

let fmod a b = a -. b *. (floor (a /. b)) ;;

let binaries = ["+", aplbinarylift (+.); "-", aplbinarylift (-.);
"*", aplbinarylift ( *. ); "/", aplbinarylift (/.);
"%", aplbinarylift fmod] ;;

let eval = eval_with unaries binaries ;;

let show_value = show_atom;;

let parse value = Toyapl_parser.parse_line Toyapl_lexer.next
(Lexing.from_string (value ^ "\n")) ;;

let repl inp outp =
while true do
output_string outp " "; flush outp;
let inval = input_line inp in
try output_string outp ((show_value (eval (parse inval))) ^ "\n");
flush outp
with Parsing.Parse_error -> output_string outp "?parse error\n"
| Op_not_found (op) -> output_string outp ("?not found " ^ op ^ "\n")
| Stack_overflow -> output_string outp "?stack overflow\n"
| Mismatched_list_lengths(a, b) ->
output_string outp ("?length mismatch: " ^ show_value a ^ ", " ^
show_value b ^ "\n")
done ;;

(* exception Test_failure of int * 'a * 'a ;; *)

let test () =
(* atoms *)
assert ([45.] = eval (Atom [45.]));
assert ("45" = show (Atom [45.]));
assert ([45.; 50.] = eval (Atom [45.; 50.]));
assert ("45 50" = show (Atom [45.; 50.]));
(* parenthesized expressions *)
let f7 = Parenthesized (Parenthesized (Atom [47.])) in
assert ("((47))" = show f7);
assert ([47.] = eval f7);
(* binary ops (depends on addition) *)
let seven = Binary ((Atom [3.]), "+", (Atom [4.])) in
assert ("3 + 4" = show seven);
assert ([7.] = eval seven);
assert ([7.] = eval (Parenthesized seven));
assert ("(3 + 4)" = show (Parenthesized seven));
(* left operands of binary ops: binary ops *)
let twelve = Binary (seven, "+", Atom [5.]) in
assert ([12.] = eval twelve);
assert ("(3 + 4) + 5" = show twelve);
assert ("(3 + 4) + 5" = show (Binary (Parenthesized seven,
"+", Atom [5.])));
(* left operands of binary ops: atoms *)
assert ("5 + 3 + 4" = show (Binary (Atom [5.], "+", seven)));
(* unary ops (depends on negation) *)
let minus_one = Binary (Atom [3.], "+", Unary("-",Atom [4.])) in
assert ("3 + - 4" = show minus_one);
assert ([-1.] = eval minus_one);
(* left operands of binary ops: unary ops *)
assert ("(- 4) + 3" = show (Binary (Unary("-", Atom [4.]),
"+", Atom [3.])));
(* no precedence given to unary ops *)
assert ("- 3 + 4" = show (Unary ("-", seven)));
assert ([-7.] = eval (Unary ("-", seven)));

(* unary ops: identity, sum, product *)
assert ([7.] = eval (Unary ("+", seven)));
assert ([202.] = eval (Unary ("+/", Atom [49.; 50.; 51.; 52.])));
assert ([0.] = eval (Unary ("+/", Atom [])));
assert ("+/ ()" = show (Unary ("+/", Atom [])));
assert ([6497400.] = eval (Unary ("*/", Atom [49.; 50.; 51.; 52.])));
assert ([1.] = eval (Unary ("*/", Atom [])));
(* simple unary ops on vectors *)
let minus_nums = Unary ("-", Atom [3.; 4.]) in
assert ([-3.; -4.] = eval minus_nums);
assert ("- 3 4" = show minus_nums);
assert ([-3.; -4.] = eval (Unary ("+", minus_nums)));
assert ("+ - 3 4" = show (Unary ("+", minus_nums)));

(* other binary ops *)
assert([-1.] = eval (Binary (Atom [3.], "-", Atom [4.])));
assert([12.] = eval (Binary (Atom [3.], "*", Atom [4.])));
assert([60.5] = eval (Binary (Atom [121.], "/", Atom [2.])));
assert([2.] = eval (Binary (Atom [122.], "%", Atom [10.])));

(* pointwise binary ops with vectors *)
assert([7.; 8.; 9.; 10.] = eval (Binary (Atom [3.], "+",
Atom [4.; 5.; 6.; 7.])));
assert([7.; 8.; 9.; 10.] = eval (Binary (Atom [4.; 5.; 6.; 7.], "+",
Atom [3.])));
assert([3.; 3.; 3.; 4.] = eval (Binary (Atom [7.; 8.; 9.; 10.], "-",
Atom [4.; 5.; 6.; 6.])));
try ignore (eval (Binary (Atom [2.; 3.], "+", Atom [5.; 6.; 7.])));
assert false with
Mismatched_list_lengths (a, b) ->
assert (([2.; 3.], [5.; 6.; 7.]) = (a, b));

(* iota *)
let iota5 = Unary ("iota", Atom [5.]) in
assert([0.; 1.; 2.; 3.; 4.] = eval iota5);
assert("iota 5" = show iota5);
assert([120.] = eval(Unary("*/", Binary(Atom [1.], "+", iota5))));

(* show_value *)
assert("0 1 2 3 4" = show_value (eval iota5));

(* parsing *)
assert(Atom [23.] = parse "23");
assert(Atom [2.;3.] = parse "2 3");
assert(Unary("+/", Unary("iota", Atom [5.])) = parse "+/iota 5");

(* total system tests *)
assert("1 2 0 1 2 0 1 2 0 1" =
(show_value (eval (parse "((iota 10) - 5) % 3"))));
assert("0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1" =
(show_value (eval (parse "(1 + iota 10) / 10"))));
assert("15 18 21" = (show_value (eval (parse "3 * 4 5 6 + 1"))));
;;
test () ;;
