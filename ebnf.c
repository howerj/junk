/** @file       bnf.c
 *  @brief      An Extended Backus–Naur Form Parser generator for C (work in progress)
 *  @author     Richard James Howe.
 *  @copyright  Copyright 2015 Richard James Howe.
 *  @license    LGPL v2.1 or later version
 *  @email      howe.r.j.89@gmail.com 
 *  See <https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_Form>*/
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOKEN_LEN	(256)

enum { 
	ERROR,
	ID,
	TERMINAL,
	LSB = '[',  RSB = ']', 
	LBR = '{',  RBR = '}', 
	LPAR = '(', RPAR = ')', 
	LBB = '<',  RBB = '>',
       	EQ = '=',
       	PIPE = '|',
	PERIOD = '.',
	COMMA = ',',
	SEMI = ';',
	END = EOF
};

static FILE *in = NULL;
static char token[TOKEN_LEN] = { 0 }, token_id = ERROR;
static int unget = 0, idx = 0;

/* util */

void fatal(char *msg) { fprintf(stderr, "%s\n", msg); abort(); }

/* lexer */

int digit(char c)  { 
	return isdigit(c); 
}

int letter(char c) { 
	return isalpha(c); 
}

int symbol(char c) { 
	return !!strchr("[]{}()<>'\"=|.,;", c); 
}

void untoken(int token) {
	if(unget)
		fatal("token already pushed back");
	unget = 1;
	token_id = token;
}

int identifier(char c) { 
	idx = 0;
	token[0] = c;
	for(;;) {
		c = getc(in);
		if(letter(c) || digit(c)) {
			token[++idx] = c;
		} else {
			ungetc(c, in);
			token[++idx] = 0;
			return ID;
		}
	}
}

int terminal(char expect) { 
	int c;
	idx = 0;
	for(;;) {
		c = getc(in);
		if(c == expect) {
			token[idx] = 0;
			return TERMINAL;
		} else {
			token[idx++] = c;
		}
	}
	return TERMINAL;
}

int lexer(void)    
{
	int c;
	if(unget) {
		unget = 0;
		return token_id;
	}
again:	c = getc(in);
	switch(c) {
	case ' ':
	case '\t':	
	case '\n': goto again;
	case '[': return LSB;
	case ']': return RSB;
	case '{': return LBR;
	case '}': return RBR;
	case '(': return LPAR;
	case ')': return RPAR;
	case '<': return LBB;
	case '>': return RBB;
	case '=': return EQ;
	case '|': return PIPE;
	case '.': return PERIOD;
	case ',': return COMMA;
	case ';': return SEMI;
	case EOF: return END;
	case '\'': 
	case '"': 
		return terminal(c);
	default:
		if(letter(c))
			return identifier(c);
		goto fail;
	}
fail:	fatal("invalid token");
	return ERROR; 
}

/*parser*/

void lhs(void) { /* identifier */ }

void rhs(void) {
       	/* rhs = identifier 
	 *       | terminal 
	 *       | "[" , rhs , "]" | "(" , rhs , ")" | "{" , rhs , "}" 
	 *       | rhs , "|" , rhs
	 *       | rhs , "," , rhs ";"  */ 
}
void rule(void) { /* rule = lhs , "=" , rhs , ";" ; */ }

void grammar(void) { /* grammar = { rule } ; */ }

/*generate C directory or compile to VM, and spit out program and VM? */

int main(int argc, char **argv) {
	if(argc < 2) {
		fprintf(stderr, "usage: %s files\n", argv[0]);
		return 1;
	}

	while(++argv, --argc) { 
		int t;
		in = fopen(argv[0], "rb");
		if(!in) {
			perror(argv[0]);
			return 1;
		}
		while((t = lexer()) != END) {
			if(t == ID) {
				printf("<ID %s>\n", token);
			} else if(t == TERMINAL) {
				printf("<TERMINAL %s>\n", token);
			} else {
				printf("%c\t%d\n", t, t);
			}
		}
		fclose(in);
	}

        return 0;
}

