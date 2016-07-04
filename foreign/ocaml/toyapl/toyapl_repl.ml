(* And here's a top-level driver for the main program, which I put in
toyapl_repl.ml: *)
print_string "Welcome to toyapl, a tiny APL subset.
Separate numbers by spaces; available ops are + - * / +/ */ % iota
% is modulo; / is division.
This program is not expected to be useful; I wrote it to learn OCaml.
" ;
try Toyapl.repl stdin stdout with End_of_file -> () ;;
