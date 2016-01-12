/** @file       bnf.c
 *  @brief      A Backus–Naur Form Parser generator for C (work in progress)
 *  @author     Richard James Howe.
 *  @copyright  Copyright 2015 Richard James Howe.
 *  @license    LGPL v2.1 or later version
 *  @email      howe.r.j.89@gmail.com **/
#include <stdio.h>

/* Parser specification 
 * From <https://en.wikipedia.org/wiki/Backus%E2%80%93Naur_Form>
 * <syntax>         ::= <rule> | <rule> <syntax>
 * <rule>           ::= <opt-whitespace> "<" <rule-name> ">" <opt-whitespace> "::=" <opt-whitespace> <expression> <line-end>
 * <opt-whitespace> ::= " " <opt-whitespace> | ""
 * <expression>     ::= <list> | <list> "|" <expression>
 * <line-end>       ::= <opt-whitespace> <EOL> | <line-end> <line-end>
 * <list>           ::= <term> | <term> <opt-whitespace> <list>
 * <term>           ::= <literal> | "<" <rule-name> ">"
 * <literal>        ::= '"' <text> '"' | "'" <text> "'" 
 */

/*parse*/

/*generate C directory or compile to VM, and spit out program and VM? */

int main(void) {
        return 0;
}

