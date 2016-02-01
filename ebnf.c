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
#include <stdarg.h>

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

static char *pstrdup(const char *s)
{
	assert(s);
	char *str;
	if (!(str = malloc(strlen(s) + 1)))
		return NULL;
	strcpy(str, s);
	return str;
}

static void pdepth(unsigned depth)
{
	while(depth--)
		putchar('\t');
}

static void ptoken(parse_state *ps, FILE *out)
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
static void *_failed(parse_state *ps, char *msg) 
{
	fprintf(stderr, "syntax error on line %u\nexpected\n\t%s\ngot\n\t", ps->line, msg);
	ptoken(ps, stderr);
	if(ps->jmpbuf_set)
		longjmp(ps->jb, 1);
	abort(); 
	return 0; 
}

static void debug(parse_state *ps, char *msg, unsigned depth) 
{ 
	if(ps->debug) {
		pdepth(depth);
		printf("%s\n", msg);
	}
}

/* lexer */

static void untoken(parse_state *ps, int token) 
{
	if(ps->unget)
		failed(ps, "token already pushed back");
	ps->unget = 1;
	ps->token_id = token;
}

static int digit(char c)  
{	/* digit = "0" | ... | "9" */
	return isdigit(c); 
}

static int letter(char c) 
{ 	/* digit = "A" | ... | "Z" | "a" ... "z" */
	return isalpha(c); 
}

static int symbol(char c) 
{	/* symbol = ... */
	return !!strchr("[]{}()<>'\"=|.,;", c); 
}

static int character(char c) 
{	/* character = letter | digit | symbol | "_" ; */
	return digit(c) || letter(c) || symbol(c) || c == '_';
}

static int identifier(parse_state *ps, char c) 
{	/* identifier = letter , { letter | digit | "_" } ; */
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

static int terminal(parse_state *ps, char expect) { 
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

static int lexer(parse_state *ps, unsigned depth)
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

enum { FAIL, GRAMMAR, RULE, LHS, RHS, TERM, GROUPING, OPTIONAL, REPETITION, LAST };

static char *names[] = {
	[FAIL] = "fail", [GRAMMAR] = "grammar", [RULE] = "rule", [LHS] = "lhs", 
	[RHS] = "rhs", [TERM] = "term", [GROUPING] = "grouping", [OPTIONAL] = "optional", 
	[REPETITION] = "repetition"
};

typedef struct node {
	char *sym;
	size_t len, allocated;
	int token_id;
	struct node *n[0];
} node;

static node *mk(int token_id, char *sym, size_t len, ...) 
{
	node *ret;
	va_list ap;
	size_t i;
	if(!(ret = calloc(1, sizeof(*ret) + (len * sizeof(ret)))))
		perror("calloc"), abort();
	ret->token_id = token_id; 
	ret->sym = sym ? pstrdup(sym) : NULL;
	ret->len = len;
	ret->allocated = len;
	va_start(ap, len);
	for(i = 0; i < len; i++)
		ret->n[i] = va_arg(ap, node*);
	va_end(ap);
	return ret;
}

static node *grow(node *n, size_t new_len)
{
	node *ret = n;
	if(new_len > n->allocated) {
		assert(new_len * 2 > n->allocated);
		if(!(ret = realloc(n, sizeof(*ret) + (new_len * 2 * sizeof(ret))))) {
			perror("realloc"); 
			abort();
		}
		ret->allocated = new_len * 2;
	} 
	return ret;
}

static void unmk(node *n)
{
	if(!n)
		return;
	free(n->sym);
	for(size_t i = 0; i < n->len; i++)
		unmk(n->n[i]);
	free(n);
}

static void print(node *n, unsigned depth)
{
	if(!n)
		return;
	pdepth(depth);
	if(names[n->token_id])
		printf("%s %s\n", names[n->token_id], n->sym ? n->sym : "");
	else if(n->token_id != EOF)
		printf("%c %s\n", n->token_id, n->sym ? n->sym : "");
	else /*special case*/
		printf("EOF\n");
	for(size_t i = 0; i < n->len; i++)
		print(n->n[i], depth+1);
}

static int accept(parse_state *ps, int token_id, unsigned depth)
{
	if(ps->token_id == token_id) {
		lexer(ps, depth);
		return 1;
	}
	return 0;
}

static node *lhs(parse_state *ps, unsigned depth) 
{
	/* lhs = identifier */
	debug(ps, "lhs", depth);
	if(ps->token_id == ID)
		return mk(LHS, 0, 1, mk(ID, ps->token, 0));
	return failed(ps, "lhs = identifier ;");
}

static node *rhs(parse_state *ps, unsigned depth);

static node *grouping(parse_state *ps, unsigned depth) 
{
	node *n; 
	if(!accept(ps, LPAR, depth))
		return NULL;
	debug(ps, "grouping", depth);
	n = mk(GROUPING, NULL, 1, NULL);	
	if((n->n[0] = rhs(ps, depth+1)))
		lexer(ps, depth);
	else
		goto fail;
	if(ps->token_id == RPAR)
		return n;
fail:	return failed(ps, "grouping = '(' , rhs , ')' ;");
}

static node *optional(parse_state *ps, unsigned depth) 
{ 
	node *n;
	if(!accept(ps, LSB, depth))
		return NULL;
	debug(ps, "optional", depth);
	n = mk(OPTIONAL, NULL, 1, NULL);	
	if((n->n[0] = rhs(ps, depth+1)))
		lexer(ps, depth);
	else
		goto fail;
	if(ps->token_id == RSB)
		return n;
fail:	return failed(ps, "optional = '[' , rhs , ']' ;");
}

static node *repetition(parse_state *ps, unsigned depth) 
{
	node *n;
	if(!accept(ps, LBR, depth))
		return NULL;
	debug(ps, "repetition", depth);
	n = mk(REPETITION, NULL, 1, NULL);	
	if((n->n[0] = rhs(ps, depth+1)))
		lexer(ps, depth);
	else
		goto fail;
	if(ps->token_id == RBR)
		return n;
fail:	return failed(ps, "repetition = '{' , rhs , '}' ;");
}

static node *term(parse_state *ps, unsigned depth) 
{
	node *n = mk(TERM, NULL, 1, NULL);	
	debug(ps, "term", depth);
	if(ps->token_id == ID || ps->token_id == TERMINAL) {
		n->n[0] = mk(ps->token_id, ps->token, 0);
		return n;
	}
	if((n->n[0] = grouping(ps, depth+1)))
		return n;
	if((n->n[0] = optional(ps, depth+1)))
		return n;
	if((n->n[0] = repetition(ps, depth+1)))
		return n;
	return failed(ps, "term = identifier | grouping | optional | repetition ;");
}

static node *rhs(parse_state *ps, unsigned depth) {
	node *t, *n = mk(RHS, NULL, 1, NULL);
	for(size_t i = 0;;) {
		if((t = term(ps, depth+1))) {
			n = grow(n, i+1);
			n->n[i] = t;
			n->len = i + 1;
			lexer(ps, depth);
		} else {
			goto fail;
		}
		if(ps->token_id == PIPE || ps->token_id == COMMA) {
			i++;
			lexer(ps, depth);
			continue;
		}
		untoken(ps, ps->token_id);
		break;
	}
	return n;
fail:	return failed(ps, "rhs = term | term , '|' rhs | term , ',' rhs ;");
}

static node *rule(parse_state *ps, unsigned depth) 
{
	node *n = mk(RULE, NULL, 2, NULL, NULL);
	debug(ps, "rule", depth);
	if((n->n[0] = lhs(ps, depth+1)))
		lexer(ps, depth);
	else
		goto fail;
	if(!accept(ps, EQ, depth))
		goto fail;
	if((n->n[1] = rhs(ps, depth+1)))
		lexer(ps, depth);
	else
		goto fail;
	if(ps->token_id == SEMI)
		return n;
fail:	return failed(ps, "rule = lhs , '=' , rhs , ';' ;");
}

static node *grammar(parse_state *ps, unsigned depth) 
{
	node *t, *n = mk(GRAMMAR, 0, 1, NULL);
	for(size_t i = 0;;i++) {
		lexer(ps, depth);
		if(ps->token_id == END)
			return n;
		if((t = rule(ps, depth + 1))) {
			n = grow(n, i+1);
			n->len = i + 1;
			n->n[i] = t;
		} else {
			goto fail;
		}
	}
fail:	return failed(ps, "grammar = EOF | { rule } ; EOF");
}

node *parse_ebnf(parse_state *ps, FILE *in, int debug_on) 
{
	assert(ps && in);
	memset(ps, 0, sizeof(*ps));
	ps->in = in;
	ps->debug = debug_on;
	ps->line = 1;
	if(setjmp(ps->jb)) {
		ps->jmpbuf_set = 0;
		return NULL;
	}
	ps->jmpbuf_set = 1;
	return grammar(ps, 0);
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
		node *n = parse_ebnf(&ps, in, 1);
		print(n, 0);
		unmk(n);
		fclose(in);
	}

        return 0;
}

