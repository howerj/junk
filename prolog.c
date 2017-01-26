#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

typedef enum { ATOM, VAR, CONS, GOAL, CLAUSE } type;

typedef struct obj {
	type t;
	int arity;
	struct obj *p[];
} obj;

#define mkatom(NAME) mk(ATOM, 1, (NAME))
#define die(MSG) _die((MSG), __FILE__, __func__, __LINE__)


void _die(const char *msg, const char *file, const char *func, unsigned line)
{
	fprintf(stderr, "die: %s in %s in function %s on line %u\n", msg, file, func, line);
	abort();
}

obj* mk(type t, int count, ...)
{
	obj *r;
	va_list ap;
	int i;
	r = calloc(1, sizeof(obj) + count * sizeof(obj*));
	r->t = t;
	va_start(ap, count);
	for(i = 0; i < count; i++) 
		r->p[i] = va_arg(ap, obj*);
	va_end(ap);
	return r;
}

int atom_equal(obj* x, obj *y)
{
	assert(x->t == ATOM && y->t == ATOM);
	return !strcmp(x->p[0], y->p[0]);
}

int print(obj* o)
{
	int i, r;
	assert(o);
	switch(o->t)
	{
	case ATOM:
		return printf("%s ", o->p[0]);
	case VAR:
		return print(o->p[0]);
	case CONS:
		r = printf("%s\n", o->p[0]);
		for(i = 0; i < o->arity; i++)
		{
			print(o->p[i]);
			puts(", ");
		}
		return r;
	case GOAL:
		print(o->p[0]);
		if(o->p[1]) {
			puts("; ");
			return print(o->p[1]);
		}
	case CLAUSE:
		print(o->p[0]);
		puts(":- ");
		if(!o->p[1])
			return puts("true");
		else 
			return print(o->p[1]);
	default:
		die("invalid type");
	}
}

int unify(obj* x, obj* y)
{
	return 0;
}

obj *copy(obj *o)
{
	assert(o);
	switch(o->t)
	{
	case VAR: 
	case CONS:
	case GOAL: 
	case CLAUSE: 
		break;
	case ATOM: 
	default:
		die("invalid type");
	}
}

void solve(obj* goal)
{
	assert(goal->t == GOAL);
}

int main(void)
{ 
	return 0;
}

