/** @file       bnf.c
 *  @brief      An Extended Backus–Naur Form Parser generator for C (work in progress)
 *  @author     Richard James Howe.
 *  @copyright  Copyright 2015 Richard James Howe.
 *  @license    LGPL v2.1 or later version
 *  @email      howe.r.j.89@gmail.com 
 *  See <https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_Form>
 *  @todo Add in comments and other parts of the EBNF standard
 *  @todo Add in escape characters
 *  @todo Allow parsing of binary files
 *  @todo Add spaces to the grammar
 *  */
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOKEN_LEN	(256)

enum { 
	LEXER_ERROR,
	ID = 'I',
	TERMINAL = 'T',
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
static char token[TOKEN_LEN] = { 0 }, token_id = LEXER_ERROR;
static int unget = 0, idx = 0, line = 1;

/* util */

#define fatal(MSG) _fatal((MSG), __LINE__)
int _fatal(char *msg, unsigned iline) { 
	fprintf(stderr, "%u,%s => %c %s\n", line, msg, token_id, 
			token_id == ID || token_id == TERMINAL ? token : ""); 
	abort(); 
	return 0; 
}

/* lexer */

void untoken(int token) {
	if(unget)
		fatal("token already pushed back");
	unget = 1;
	token_id = token;
}

int digit(char c)  { /* digit = "0" | ... | "9" */
	return isdigit(c); 
}

int letter(char c) {  /* digit = "A" | ... | "Z" | "a" ... "z" */
	return isalpha(c); 
}

int symbol(char c) { /* symbol = ... */
	return !!strchr("[]{}()<>'\"=|.,;", c); 
}

int character(char c) { /* character = letter | digit | symbol | "_" ; */
	return digit(c) || letter(c) || symbol(c) || c == '_';
}

int identifier(char c) { /* identifier = letter , { letter | digit | "_" } ; */
	idx = 0;
	token[0] = c;
	for(;;) {
		c = getc(in);
		if(letter(c) || digit(c) || c == '_') {
			token[++idx] = c;
		} else {
			ungetc(c, in);
			token[++idx] = 0;
			return ID;
		}
	}
}

int terminal(char expect) { 
	/* terminal	= "'" , character , { character } , "'" 
			| '"' , character , { character } , '"' ; */
	int c;
	idx = 0;
	for(;;) {
		c = getc(in);
		if(c == expect) {
			token[idx] = 0;
			return TERMINAL;
		} else if(character(c)) { 
			token[idx++] = c;
		} else {
			fatal("incorrect terminal");
		}
	}
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
	case '\n': line++;
	case ' ':
	case '\t': goto again;	
	case '[': return token_id = LSB;
	case ']': return token_id = RSB;
	case '{': return token_id = LBR;
	case '}': return token_id = RBR;
	case '(': return token_id = LPAR; /** @todo add in "(*" and "*)" comments here*/
	case ')': return token_id = RPAR;
	case '<': return token_id = LBB;
	case '>': return token_id = RBB;
	case '=': return token_id = EQ;
	case '|': return token_id = PIPE;
	case '.': return token_id = PERIOD;
	case ',': return token_id = COMMA;
	case ';': return token_id = SEMI;
	case EOF: return token_id = END;
	case '\'': 
	case '"': 
		return token_id = terminal(c);
	default:
		if(letter(c)) 
			return token_id = identifier(c);
		goto fail;
	}
fail:	return fatal("invalid token");
}

/*parser*/

enum { FAIL, GRAMMAR, RULE, LHS, RHS, TERM, GROUPING, OPTIONAL, REPETITION };

/* make struct for parsed type and duplicate token */

int lhs(void) { /* identifier */ 
	if(token_id == ID)
		return LHS;
fail: return fatal("lhs");
}

int grouping(void) { /* "(" , rhs , ")"  */ 
	if(token_id == LPAR)
		lexer();
	else
		return FAIL;
	if(rhs() == RHS)
		lexer();
	else
		goto fail;
	if(token_id == RPAR)
		return GROUPING;
fail:	return fatal("grouping");
}

int optional(void) { /* "[" , rhs , "]" */  
	if(token_id == LSB)
		lexer();
	else
		return FAIL;
	if(rhs() == RHS)
		lexer();
	else
		goto fail;
	if(token_id == RSB)
		return GROUPING;
fail:	return fatal("optional");
}

int repetition(void) { /* "{" , rhs , "}" */ 
	if(token_id == LBR)
		lexer();
	else
		return FAIL;
	if(rhs() == RHS)
		lexer();
	else
		goto fail;
	if(token_id == RBR)
		return REPETITION;
fail:	return fatal("repetition");
}

int term(void) {
	if(token_id == ID || token_id == TERMINAL)
		return TERM;
	if(grouping() == GROUPING)
		return TERM;
	if(optional() == OPTIONAL)
		return TERM;
	if(repetition() == REPETITION)
		return TERM;
fail:	return fatal("term");
}

int rhs(void) {
       	/* rhs =   term 
	 *       | term , "|" , rhs
	 *       | term , "," , rhs ;  */ 
again:	if(term() == TERM)
		lexer();
	else
		goto fail;
	if(token_id == PIPE || token_id == COMMA) {
		lexer();
		goto again;
	}
	untoken(token_id);
	return RHS;
fail:	return fatal("rhs");
}

int rule(void) { /* rule = lhs , "=" , rhs , ";" ; */
	if(lhs() == LHS)
		lexer();
	else
		goto fail;
	if(token_id == EQ)
		lexer();
	else
		goto fail;
	if(rhs() == RHS)
		lexer();
	else
		goto fail;
	if(token_id == SEMI)
		return RULE;
fail:	return fatal("rule");
}

int grammar(void) { /* grammar = { rule } ; */ 
	for(;;) {
		lexer();
		if(token_id == END)
			return GRAMMAR;
		if(rule() != RULE)
			goto fail;
		
	}
fail:	return fatal("grammar");
}

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
		grammar();
		/*
		while((t = lexer()) != END) {
			if(t == ID) {
				printf("<ID %s>\n", token);
			} else if(t == TERMINAL) {
				printf("<TERMINAL %s>\n", token);
			} else {
				printf("%c\t%d\n", t, t);
			}
		}*/
		fclose(in);
	}

        return 0;
}

