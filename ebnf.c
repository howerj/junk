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
 *  @todo Build up parse tree
 *  @todo Clean up and simplify code
 *  */
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

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

typedef struct parse_state {
	FILE *in;
	unsigned line, unget, idx, debug, jmpbuf_set;
	int token_id;
	char token[TOKEN_LEN];
	jmp_buf jb;
} parse_state;

/* util */
void pdepth(unsigned depth)
{
	while(depth--)
		putchar('\t');
}

void ptoken(parse_state *ps, FILE *out)
{
	int token = ps->token_id;
	fprintf(out, "%s ", ps->unget ? "unget-token" : "token");
	if(token == ID)
		fprintf(out, "<ID %s>", ps->token);
	else if(token == TERMINAL)
		fprintf(out, "<TERMINAL \"%s\">", ps->token);
	else if(token == END)
		fprintf(out, "<EOF>");
	else
		fputc(token, out);
	fputc('\n', out);
}

#define failed(PS, MSG) _failed((PS), (MSG))
int _failed(parse_state *ps, char *msg) {
	fprintf(stderr, "syntax error on line %u\nexpected\n\t%s\ngot\n\t", ps->line, msg);
	ptoken(ps, stderr);
	if(ps->jmpbuf_set)
		longjmp(ps->jb, 1);
	abort(); 
	return 0; 
}

void debug(parse_state *ps, char *msg, unsigned depth) 
{ 
	if(ps->debug) {
		pdepth(depth);
		printf("%s\n", msg);
	}
}

/* lexer */

void untoken(parse_state *ps, int token) {
	if(ps->unget)
		failed(ps, "token already pushed back");
	ps->unget = 1;
	ps->token_id = token;
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

int identifier(parse_state *ps, char c) { /* identifier = letter , { letter | digit | "_" } ; */
	ps->idx = 0;
	ps->token[0] = c;
	for(;;) {
		c = getc(ps->in);
		assert(ps->idx < TOKEN_LEN - 1);
		if(letter(c) || digit(c) || c == '_') {
			ps->token[++(ps->idx)] = c;
		} else {
			ungetc(c, ps->in);
			ps->token[++(ps->idx)] = 0;
			return ID;
		}
	}
}

int terminal(parse_state *ps, char expect) { 
	ps->idx = 0;
	for(;;) {
		int c = getc(ps->in);
		assert(ps->idx < TOKEN_LEN - 1);
		if(c == expect) {
			ps->token[ps->idx] = 0;
			return TERMINAL;
		} else if(character(c)) { 
			ps->token[(ps->idx)++] = c;
		} else {
			failed(ps, "terminal = \"'\" character , { character } , \"'\""
				   " | '\"' , character , { character } , '\"' ;");
		}
	}
}

int lexer(parse_state *ps, unsigned depth)
{
	int c;
	if(ps->unget) {
		ps->unget = 0;
		if(ps->debug) {
			pdepth(depth);
			ptoken(ps, stdout);
		}
		return ps->token_id;
	}
again:	c = getc(ps->in);
	switch(c) {
	case '\n': ps->line++;
	case ' ':
	case '\t': goto again;	
	case '[':  ps->token_id = LSB;	break;
	case ']':  ps->token_id = RSB;	break;
	case '{':  ps->token_id = LBR;	break;
	case '}':  ps->token_id = RBR;	break;
	/** @todo add in "(*" and "*)" comments here*/
	case '(':  ps->token_id = LPAR; 	break;
	case ')':  ps->token_id = RPAR;	break;
	case '<':  ps->token_id = LBB;	break;
	case '>':  ps->token_id = RBB;	break;
	case '=':  ps->token_id = EQ;	break;
	case '|':  ps->token_id = PIPE;	break;
	case '.':  ps->token_id = PERIOD;	break;
	case ',':  ps->token_id = COMMA;	break;
	case ';':  ps->token_id = SEMI;	break;
	case EOF:  ps->token_id = END;	break;
	case '\'': 
	case '"':  ps->token_id = terminal(ps, c); break;
	default:
		if(letter(c)) {
			ps->token_id = identifier(ps, c);
			break;
		}
		failed(ps, "identifier = letter , { letter | digit | '_' } ;");
	}
	if(ps->debug) {
		pdepth(depth);
		ptoken(ps, stdout);
	}
	return ps->token_id;
}

/*parser*/

enum { FAIL, GRAMMAR, RULE, LHS, RHS, TERM, GROUPING, OPTIONAL, REPETITION };

/* make struct for parsed type and duplicate token */

int lhs(parse_state *ps, unsigned depth) 
{ 
	/* lhs = identifier */
	debug(ps, "lhs", depth);
	if(ps->token_id == ID)
		return LHS;
	return failed(ps, "lhs = identifier ;");
}

int rhs(parse_state *ps, unsigned depth);

int grouping(parse_state *ps, unsigned depth) 
{ 
	if(ps->token_id == LPAR)
		lexer(ps, depth);
	else
		return FAIL;
	debug(ps, "grouping", depth);
	if(rhs(ps, depth+1) == RHS)
		lexer(ps, depth);
	else
		goto fail;
	if(ps->token_id == RPAR)
		return GROUPING;
fail:	return failed(ps, "grouping = '(' , rhs , ')' ;");
}

int optional(parse_state *ps, unsigned depth) 
{ 
	if(ps->token_id == LSB)
		lexer(ps, depth);
	else
		return FAIL;
	debug(ps, "optional", depth);
	if(rhs(ps, depth+1) == RHS)
		lexer(ps, depth);
	else
		goto fail;
	if(ps->token_id == RSB)
		return GROUPING;
fail:	return failed(ps, "optional = '[' , rhs , ']' ;");
}

int repetition(parse_state *ps, unsigned depth) { 
	/* repetition =  "{" , rhs , "}" ; */ 
	if(ps->token_id == LBR)
		lexer(ps, depth);
	else
		return FAIL;
	debug(ps, "repetition", depth);
	if(rhs(ps, depth+1) == RHS)
		lexer(ps, depth);
	else
		goto fail;
	if(ps->token_id == RBR)
		return REPETITION;
fail:	return failed(ps, "repetition = '{' , rhs , '}' ;");
}

int term(parse_state *ps, unsigned depth) 
{
	debug(ps, "term", depth);
	if(ps->token_id == ID || ps->token_id == TERMINAL)
		return TERM;
	if(grouping(ps, depth+1) == GROUPING)
		return TERM;
	if(optional(ps, depth+1) == OPTIONAL)
		return TERM;
	if(repetition(ps, depth+1) == REPETITION)
		return TERM;
	return failed(ps, "term = identifier | grouping | optional | repetition ;");
}

int rhs(parse_state *ps, unsigned depth) {
again:	debug(ps, "rhs", depth);
	if(term(ps, depth+1) == TERM)
		lexer(ps, depth);
	else
		goto fail;
	if(ps->token_id == PIPE || ps->token_id == COMMA) {
		lexer(ps, depth);
		goto again;
	}
	untoken(ps, ps->token_id);
	return RHS;
fail:	return failed(ps, "rhs = term | term , '|' rhs | term , ',' rhs ;");
}

int rule(parse_state *ps, unsigned depth) { 
	debug(ps, "rule", depth);
	if(lhs(ps, depth+1) == LHS)
		lexer(ps, depth);
	else
		goto fail;
	if(ps->token_id == EQ)
		lexer(ps, depth);
	else
		goto fail;
	if(rhs(ps, depth+1) == RHS)
		lexer(ps, depth);
	else
		goto fail;
	if(ps->token_id == SEMI)
		return RULE;
fail:	return failed(ps, "rule = lhs , '=' , rhs , ';' ;");
}

int grammar(parse_state *ps, unsigned depth) 
{
	for(;;) {
		lexer(ps, depth);
		if(ps->token_id == END)
			return GRAMMAR;
		if(rule(ps, depth + 1) != RULE)
			goto fail;
		
	}
fail:	return failed(ps, "grammar = EOF | { rule } ; EOF");
}

int parse_ebnf(parse_state *ps, FILE *in, int debug_on) 
{
	assert(ps && in);
	memset(ps, 0, sizeof(*ps));
	ps->in = in;
	ps->debug = debug_on;
	ps->line = 1;
	if(setjmp(ps->jb)) {
		ps->jmpbuf_set = 0;
		return -1;
	}
	ps->jmpbuf_set = 1;
	return grammar(ps, 0) == GRAMMAR;
}

/*generate C directory or compile to VM, and spit out program and VM? */

static parse_state ps;

int main(int argc, char **argv) {
	if(argc < 2) {
		fprintf(stderr, "usage: %s files\n", argv[0]);
		return 1;
	}

	while(++argv, --argc) { 
		FILE *in = fopen(argv[0], "rb");
		if(!in) {
			perror(argv[0]);
			return 1;
		}
		parse_ebnf(&ps, in, 1);
		fclose(in);
	}

        return 0;
}

