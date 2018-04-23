/* LISP as a register machine. Any obj not in a reg or on the stack may be
 * collected at the end of eval (). */
#include <ctype.h>
#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef enum { no } truth;
enum type { nil_type, int_type, sym_type, cons_type, str_type };
typedef struct obj {
	enum type type;
	union {
		int num;
		const char *sym;
		struct { struct obj *car, *cdr; } c;
		struct { const char *s; size_t len; struct obj *super; } str;
		} x;
	struct obj *next;
	enum { free_mark, working_mark, used_mark } mark;
} obj;
/* AX accumulator -- holds the result of a an xlatression
 * DX data -- auxiliary
 * EP environment pointer -- holds the symbol table (most recent first)
 * SP stack pointer -- points to the last object pushed on the stack */
obj *ax, *dx, *ep, *stack [1024], **sp = stack - 1;
obj *nil, *t;

char tok [512];
enum tok_type { no_tok, int_tok, punct_tok, id_tok, str_tok } tok_type;
int thisc, nextc;
FILE *src;

obj *free_pool, *used_pool, *work_pool;
unsigned working_count;
truth want_collection;

obj *car (obj *x) { return x->type == cons_type ? x->x.c.car : x; }
obj *cdr (obj *x) { return x->type == cons_type ? x->x.c.cdr : nil; }
obj *caar (obj *x) { return car (car (x)); }
obj *second (obj *x) { return car (cdr (x)); }
obj *third (obj *x) { return car (cdr (cdr (x))); }

char readc (void) {
	thisc = nextc;
	do nextc = fgetc (src); while (nextc == '\r');
	if (nextc == EOF) nextc = '\0';
	return (char) thisc;
}

enum tok_type readtok (void) {
	static const char *punct = "().',; \t\n"; /* all non-id characters */
	char *ptok = tok;
	for (;;) /* Ignore spaces */
		if (isspace (nextc))
			readc ();
		else if (nextc == ';') /* A comment */
			while (nextc and nextc != '\n')
				readc ();
		else
			break;
	if (not nextc) /* End-of-file */
		tok_type = no_tok;
	else if (isdigit (nextc)) { /* Number */
		tok_type = int_tok;
		while (isdigit (nextc))
			*ptok ++ = readc ();
	}
	else if (strchr (punct, nextc)) { /* Punctuation */
		tok_type = punct_tok;
		*ptok ++ = readc ();
	}
	else if (nextc == '"') { /* String */
		tok_type = str_tok;
		readc ();
		while (readc () != '"')
			if (thisc == '\\') /* C escape sequences */
				switch (readc ()) {
				case 't': *ptok ++ = '\t'; break;
				case 'n': *ptok ++ = '\n'; break;
				default: *ptok ++ = (char) thisc; break;
				}
			else
				*ptok ++ = (char) thisc;
	}
	else { /* ID */
		tok_type = id_tok;
		while (not strchr (punct, nextc))
			*ptok ++ = (char) readc ();
	}
	*ptok = '\0'; /* nul-terminate the token */
	return tok_type;
}

void print (const char *before, obj *o, const char *after) {
	printf ("%s", before);
	switch (o->type) {
	case nil_type:
		printf ("()");
		break;
	case int_type:
		printf ("%d", o->x.num);
		break;
	case sym_type:
		printf ("%s", o->x.sym);
		break;
	case str_type:
		printf ("%.*s", o->x.str.len, o->x.str.s);
		break;
	case cons_type:
		putchar ('(');
		for ( ; o->type == cons_type; o = cdr (o))
			print ("", car (o), cdr (o)->type ? " " : "");
		if (o->type)
			print (". ", o, "");
		putchar (')');
		break;
	}
	printf ("%s", after);
}

obj *change_lists (obj **p_oldlist, obj **p_newlist) {
	obj *o = *p_oldlist;
	*p_oldlist = o->next;
	o->next = *p_newlist;
	return *p_newlist = o;
}

void mark_working (obj *o) {
	if (not o or o->mark != free_mark)
		return;
	o->mark = working_mark;
	++ working_count;
}

void mark (obj *o) {
	if (not o)
		return;
	if (o->mark == used_mark) /* Already processed */
		return;
	if (o->mark == working_mark)
		-- working_count;
	o->mark = used_mark;
	/* mark reachable objects; don't visit them recursively because
	 * traversing a long list will blow the stack. */
	if (o->type == cons_type) {
		mark_working (car (o));
		mark_working (cdr (o));
	}
	else if (o->type == str_type)
		mark_working (o->x.str.super);
}

void collect (void) {
	obj *c, **pp;
	work_pool = used_pool;
	want_collection = no;
	used_pool = NULL;
	/* All registers are reachable */
	mark_working (ax);
	mark_working (dx);
	mark_working (ep);
	for (pp = sp; pp >= stack; -- pp)
		mark_working (*pp);
	/* Mark objects reachable from other objects. */
	while (working_count)
		for (c = work_pool; c; c = c->next)
			if (c->mark == working_mark)
				mark (c);
	/* Objects not marked are garbage; reset all marks */
	while (work_pool)
		if (work_pool->mark == free_mark) {
			if (work_pool->type == sym_type)
				free ((void *) work_pool->x.sym);
			else if (work_pool->type == str_type and not work_pool->x.str.super)
				free ((void *) work_pool->x.str.s);
			change_lists (& work_pool, & free_pool);
		} else
			change_lists (& work_pool, & used_pool);
	/* Reset marks */
	for (c = used_pool; c; c = c->next)
		c->mark = free_mark;
}

obj *make (enum type type) {
	obj *o;
	if (not free_pool) { /* Exhausted free pool, so increase the pool size */
		unsigned i;
		for (i = 0; i < 10; ++ i) {
			o = calloc (1, sizeof *free_pool);
			change_lists (& o, & free_pool);
		}
		want_collection = not no;
	}
	o = free_pool;
	o->type = type;
	return change_lists (& free_pool, & used_pool);
}

obj *make_int (int num) {
	obj *o = make (int_type);
	o->x.num = num;
	return o;
}

obj *make_sym (const char *your_str) {
	obj *o;
	for (o = used_pool; o; o = o->next) /* find existing symbol */
		if (o->type == sym_type and not strcmp (o->x.sym, your_str))
			return o;
	o = make (sym_type);
	o->x.sym = strcpy (malloc (strlen (your_str) + 1), your_str);
	return o;
}

obj *make_str (const char *src, size_t len, obj *super) {
	obj *o = make (str_type);
	o->x.str.s = super ? src : strncpy (malloc (len + 1), src, len);
	o->x.str.len = len;
	o->x.str.super = super;
	return o;
}

obj *cons (obj *car, obj *cdr) {
	obj *o = make (cons_type);
	o->x.c.car = car;
	o->x.c.cdr = cdr;
	return o;
}

obj *read (void);
obj *read_list (void) { /* read in a list of lisp expressions */
	obj sentinel, **p_link = & sentinel.x.c.cdr;
	while (readtok ()) {
		if (tok_type == punct_tok and *tok == ')') {
			*p_link = nil;
			break;
		}
		if (tok_type == punct_tok and *tok == '.') {
			readtok ();
			*p_link = read ();
			readtok (); /* ASSUMES THIS IS THE ) TOKEN */
			break;
		}
		*p_link = cons (read (), nil);
		p_link = & (*p_link)->x.c.cdr;
	}
	return sentinel.x.c.cdr;
}

obj *read (void) { /* read in a lisp expression */
	switch (tok_type) {
	case no_tok: return nil;
	case int_tok:
		return make_int (atoi (tok));
	case punct_tok:
		if (*tok == '(')
			return read_list ();
		if (*tok == '\'' or *tok == ',') {
			char *id = *tok == ',' ? "unquote" : "quote";
			readtok ();
			return cons (make_sym (id), cons (read (), nil));
		}
		return nil;
	case id_tok:
		return make_sym (tok);
	case str_tok:
		return make_str (tok, strlen (tok), NULL);
	}
	return nil;
}

truth eqv (obj *a, obj *b) { /* Shallow compare */
	if (a->type != b->type)
		return no;
	switch (a->type) {
	case nil_type: return not no;
	case int_type: return a->x.num == b->x.num;
	case sym_type: return a->x.sym == b->x.sym;
	case str_type: return a->x.str.s == b->x.str.s
		and a->x.str.len == b->x.str.len;
	case cons_type: return car (a) == car (b) and cdr (a) == cdr (b);
	}
	return no;
}

truth equal (obj *a, obj *b) { /* Deep compare */
	if (a->type != b->type)
		return no;
	if (a->type == str_type and a->x.str.len == b->x.str.len)
		return not strncmp (a->x.str.s, b->x.str.s, (size_t) a->x.str.len);
	if (a->type == cons_type)
		return equal (car (a), car (b)) and equal (cdr (a), cdr (b));
	return eqv (a, b);
}

obj *find (obj *id) { /* find a variable; return it's entry (x val) or nil */
	obj *e;
	for (e = ep; e->type and not eqv (caar (e), id); e = cdr (e)) ;
	return car (e); /* returns nil if not found car (nil) => nil */
}

obj *def (obj *id, obj *val) { /* define a variable */
	return ep = cons (cons (id, cons (val, nil)), ep);
}

#define UNARY(todo) { \
	eval (second (o)); \
	todo; }
#define BINARY(todo) { \
	eval (second (o)); \
	* ++ sp = ax; \
	eval (third (o)); \
	dx = *sp --; \
	todo; }
#define TAILRECURSE(T) { o = T; goto reenter; }
#define SPECIAL(O, ID)(car (O)->type==sym_type && !strcmp (car (O)->x.sym, ID))
#define EVAL(O) (eval (O), ax)
char *buffer_str (size_t len, const char *s, size_t n) {
	static char *buf = NULL;
	static size_t max = 0;
	while (n + len >= max) {
		max = (max + 1) * 2 - 1;
		buf = realloc (buf, max + 1);
	}
	strncpy (buf + len, s, n);
	return buf;
}

void error (truth cond, obj *o, const char *msg) {
	if (not cond)
		return;
	print ("\nERROR: ", o, msg);
	exit (EXIT_FAILURE);
}

void eval (obj *o) {
	obj *old_ep = NULL, **old_sp = sp;
	reenter: /* tail recursion re-entry point */
	switch (o->type) {
	case nil_type:
	case int_type:
	case str_type:
		ax = o;
		break;
	case sym_type:
		ax = find (o); /* return symbol if found or nil */
		error (not ax->type, o, " variable is not defined");
		ax = second (ax); /* get the value of the symbol if found */
		break;
	case cons_type:
		if (SPECIAL (o, "def/sym"))
			BINARY (def (dx, ax))
		else if (SPECIAL (o, "cond")) {
			for (ax = nil, o = cdr (o); o->type; o = cdr (o))
				if (EVAL (caar (o))->type)
					TAILRECURSE (car (cdr (car (o))))
		} else if (SPECIAL (o, "prog")) {
			for (ax = nil, o = cdr (o); cdr (o)->type; o = cdr (o))
				eval (car (o));
			TAILRECURSE (car (o));
		}
		else if (SPECIAL (o, "quote"))
			ax = second (o);
		else if (SPECIAL (o, "car"))
			UNARY (ax = car (ax))
		else if (SPECIAL (o, "cdr"))
			UNARY (ax = cdr (ax))
		else if (SPECIAL (o, "setcar"))
			BINARY ( dx->x.c.car = ax; ax = dx )
		else if (SPECIAL (o, "setcdr"))
			BINARY ( dx->x.c.cdr = ax; ax = dx )
		else if (SPECIAL (o, "cons"))
			BINARY (ax = cons (dx, ax))
		else if (SPECIAL (o, "equal"))
			BINARY (ax = equal (dx, ax) ? t : nil)
		else if (SPECIAL (o, "eqv"))
			BINARY (ax = eqv (dx, ax) ? t : nil)
		else if (SPECIAL (o, "+"))
			BINARY (ax = make_int (dx->x.num + ax->x.num))
		else if (SPECIAL (o, "-"))
			BINARY (ax = make_int (dx->x.num - ax->x.num))
		else if (SPECIAL (o, "*"))
			BINARY (ax = make_int (dx->x.num * ax->x.num))
		else if (SPECIAL (o, "/"))
			BINARY (ax = make_int (dx->x.num / ax->x.num))
		else if (SPECIAL (o, "prn"))
			for (ax = nil, o = cdr (o); o->type; o = cdr (o))
				print ("", EVAL (car (o)), "");
		else if (SPECIAL (o, "countstr"))
			UNARY (ax = make_int ((int) ax->x.str.len))
		else if (SPECIAL (o, "substr"))
			BINARY ({
			(* ++ sp = dx, dx = ax);
			eval (third (cdr (o)));
			ax = make_str ((**sp).x.str.s + dx->x.num,
				(size_t) ax->x.num - dx->x.num, *sp);
			-- sp; })
		else if (SPECIAL (o, "findstr"))
			BINARY ({
			const char *p = dx->x.str.s;
			const char *c = ax->x.str.s;
			while ((p = memchr (p, *c, dx->x.str.len)) != NULL
				and strncmp (p, c, ax->x.str.len)) ;
			ax = p ? make_int (p - dx->x.str.s) : nil; })
		else if (SPECIAL (o, "joinstr")) {
			size_t len = 0;
			for (o = cdr (o); o->type; o = cdr (o), len += ax->x.str.len) {
				eval (car (o)); /* order-dependent; dont use EVAL below */
				buffer_str (len, ax->x.str.s, ax->x.str.len);
			}
			ax = make_str (buffer_str (0, NULL, 0), len, NULL);
		}
		else if (SPECIAL (o, "list2str"))
			UNARY ({
			size_t len = 0;
			for (* ++ sp = o = ax; o->type; o = cdr (o), ++ len)
				buffer_str (len, (const char *) & EVAL (car (o))->x.num, 1);
			ax = make_str (buffer_str (0, NULL, 0), len, NULL);
			-- sp; })
		else if (SPECIAL (o, "str2list"))
			UNARY ( {
			obj *sentinel = cons (nil, nil);
			obj **p_link = & sentinel->x.c.cdr;
			size_t i;
			dx = sentinel;
			for (i = 0; i < ax->x.str.len; ++ i, p_link = & (**p_link).x.c.cdr)
				*p_link = cons (make_int ((int) ax->x.str.s [i]),  nil);
			ax = cdr (sentinel); })
		else if (SPECIAL (o, "lambda"))
			ax = cdr (o);
		else if (SPECIAL (o, "set/sym"))
			BINARY (cdr (find (dx))->x.c.car = ax)
		else if (SPECIAL (o, "typeof"))
			UNARY (ax = make_int ((int) ax->type))
		else if (SPECIAL (o, "eval"))
			UNARY ( TAILRECURSE (ax) )
		else { /* User function */
			obj *body, *p, *arg = cdr (o), **link;
			old_ep = ep; /* must be before eval head; it could be a def */
			* ++ sp = EVAL (car (o)); /* Protect inline lambdas from GC */
			body = second (ax);
			p = car (ax);
			error (ax->type != cons_type, ax, " is not a function");
			for (; p->type && !SPECIAL (p, "&"); p = cdr (p), arg = cdr (arg))
				def (car (p), EVAL (car (arg)));
			if (SPECIAL (p, "&")) {
				* ++ sp = cons (nil, nil);
				link = & (**sp).x.c.cdr;
				for ( ; arg->type; arg = cdr (arg), link = & (**link).x.c.cdr)
					*link = cons (EVAL (car (arg)), nil);
				def (car (p), cdr (*sp --));
			}
			TAILRECURSE (body);
		}
		break;
	}
	if (want_collection)
		collect ();
	ep = old_ep ? old_ep : ep; /* restore env after fun body */
	sp = old_sp;
}

obj *xlat (truth quoting, obj *c) { /* translate macros */
	obj *v;
	if (c->type == sym_type and (v = find (c))->type)
		return ax = second (v); /* macro variable */
	if (c->type != cons_type) /* anything that's not a symbol or call */
		return ax = c;
	if (SPECIAL (c, "macro")) /* define a macro; remove from output code */
		return def (second (c), cdr (cdr (c))), ax = nil;
	if (SPECIAL (c, "unquote")) /* force evaluation now */
		return eval (* ++ sp = xlat (no, second (c))), -- sp, ax;
	if (not quoting and (v = find (car (c)))->type) { /* macro call */
		obj *old_ep = ep, *p = car (second (v)), *a = cdr (c);
		for ( ; a->type and not SPECIAL (p, "&"); a = cdr (a), p = cdr (p))
			def (car (p), car (a));
		if (SPECIAL (p, "&"))
			def (car (p), a);
		return eval (xlat (no, second (second (v)))), ep = old_ep, ax;
	}
	quoting = quoting or SPECIAL (c, "quote");
	* ++ sp = xlat (quoting, car (c)); /* force left-to-right order */
	return ax = cons (*sp, xlat (quoting, cdr (c))), -- sp, ax;
}

int main (int argc, char **argv) {
	ep = nil = make (nil_type);
	for (++ argv; argc > 1; ++ argv, -- argc) {
		src = fopen (*argv, "r");
		readc ();
		*sp = xlat (no, * ++ sp = cons (make_sym ("prog"), read_list ()));
		fclose (src);
		def (make_sym ("nil"), nil);
		def (t = make_sym ("t"), make_sym ("t"));
		eval (*sp);
		-- sp; /* Do not combine with previous line; -- order is not defined */
	}
	return EXIT_SUCCESS;
}
