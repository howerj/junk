/** @file       bnf.c
 *  @brief      An Extended Backus–Naur Form Parser generator for C (work in progress)
 *  @author     Richard James Howe.
 *  @copyright  Copyright 2015 Richard James Howe.
 *  @license    LGPL v2.1 or later version
 *  @email      howe.r.j.89@gmail.com **/
#include <stdio.h>

/* Parser specification:
 * <https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_Form>
	letter = "A" | "B" | "C" | "D" | "E" | "F" | "G"
	       | "H" | "I" | "J" | "K" | "L" | "M" | "N"
	       | "O" | "P" | "Q" | "R" | "S" | "T" | "U"
	       | "V" | "W" | "X" | "Y" | "Z" | "a" | "b"
	       | "c" | "d" | "e" | "f" | "g" | "h" | "i"
	       | "j" | "k" | "l" | "m" | "n" | "o" | "p"
	       | "q" | "r" | "s" | "t" | "u" | "v" | "w"
	       | "x" | "y" | "z" ;
	digit = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
	symbol = "[" | "]" | "{" | "}" | "(" | ")" | "<" | ">"
	       | "'" | '"' | "=" | "|" | "." | "," | ";" ;
	character = letter | digit | symbol | "_" ;
	 
	identifier = letter , { letter | digit | "_" } ;
	terminal = "'" , character , { character } , "'" 
		 | '"' , character , { character } , '"' ;
	 
	lhs = identifier ;
	rhs = identifier
	     | terminal
	     | "[" , rhs , "]"
	     | "{" , rhs , "}"
	     | "(" , rhs , ")"
	     | rhs , "|" , rhs
	     | rhs , "," , rhs ;

	rule = lhs , "=" , rhs , ";" ;
	grammar = { rule } ;

 */

/*parse*/



/*generate C directory or compile to VM, and spit out program and VM? */

int main(void) {
        return 0;
}

