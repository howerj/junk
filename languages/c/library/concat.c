/**@author Richard James Howe
 * @license MIT */
#include "system.h"

/**@brief duplicate or copy a string
 * @param s, NUL terminated string to copy
 * @return copy of string, NULL on failure*/
char *duplicate(const char *s)
{
	assert(s);
	const size_t l = strlen(s);
	if(l+1 < l)
		return NULL;
	char *r = malloc(l+1);
	if(!r)
		return NULL;
	r[l] = 0;
	return memcpy(r, s, l);
}

/**@brief concatenate two NUL terminated strings together
 * @param s1, first part of string
 * @param s2, second part of string
 * @return NULL on failure, pointer to NUL terminated string
 * containing s1 then s2 */
char *concatenate(const char *s1, const char *s2)
{
	assert(s1 && s2);
	const size_t l1 = strlen(s1), l2 = strlen(s2);
	const size_t t = l1 + l2;
	if(t < l1 || (t + 1) < t)
		return NULL;
	char *r = malloc(t + 1);
	if(!r)
		return NULL;
	memcpy(r,    s1, l1);
	memcpy(r+l1, s2, l2);
	r[t] = 0;
	return r;
}

#define JOIN_MAX_PARAM (128u)

/**@brief Join together a variable number of strings together
 * @param s1,  first string argument, may be NULL
 * @param ..., NULL terminated parameter list of strings
 * @return NULL on failure, or NUL terminated string containing
 * all arguments concatenated together, in order.*/
char *_join_internal(const char *s1, ...)
{
	if(!s1)
		return calloc(1, 1);
	size_t argc = 1;
	char *r = NULL;
	va_list ap;

	va_start(ap, s1);
	for(char *cur; (cur = va_arg(ap, char*));)
	       argc++;
	va_end(ap);

	if(argc > JOIN_MAX_PARAM)
		return NULL;

	assert(argc > 0);
	size_t length = strlen(s1);
	size_t i = 1, lengths[argc]; /* valid use of variadic array! */
	bool overflow = false;
	lengths[0] = length;

	va_start(ap, s1);
	for(char *cur; (cur = va_arg(ap, char*)); i++) {
		const size_t l = strlen(cur);
		lengths[i] = l;
		if((length + l) < length) {
			overflow = true;
			break;
		}
		length += l;
	}
	va_end(ap);
	if(overflow)
		return NULL;

	r = malloc(length + 1);
	if(!r)
		return NULL;
	r[length] = 0;

	size_t l = lengths[0];
	memcpy(r, s1, l);
	va_start(ap, s1);
	i = 1;
	for(char *cur = r; (cur = va_arg(ap, char*));i++) {
		const char slen = lengths[i];
		memcpy(r+l, cur, slen);
		l += slen;
	}
	va_end(ap);
	return r;
}

#define join(...) _join_internal(__VA_ARGS__, NULL)


/* NB. run through valgrind, should report no errors */

#define dup_test(E, S)\
	do {\
		char *r = duplicate((S));\
		if(!r)\
			exit(EXIT_FAILURE);\
		const bool pass = !strcmp((E), r);\
		printf("dup(\"%s\") %s \"%s\"\n", r, pass ? "==" : "<>", E);\
		if(!pass)\
			exit(EXIT_FAILURE);\
		free(r);\
	} while(0)

#define join_test(E, ...)\
	do {\
		char *r = join(__VA_ARGS__);\
		if(!r)\
			exit(EXIT_FAILURE);\
		const bool pass = !strcmp((E), r);\
		printf("join(\"%s\") %s \"%s\"\n", r, pass ? "==" : "<>", E);\
		if(!pass)\
			exit(EXIT_FAILURE);\
		free(r);\
	} while(0)

#define concat_test(E, A, B) \
	do {\
		char *r = concatenate((A), (B));\
		if(!r)\
			exit(EXIT_FAILURE);\
		const bool pass = !strcmp((E), r);\
		printf("con(\"%s\") %s \"%s\"\n", r, pass ? "==" : "<>", E);\
		if(!pass)\
			exit(EXIT_FAILURE);\
		free(r);\
	} while(0)

int main(void)
{
	/* test  expected  arg */
	dup_test("",       "");
	dup_test("a",      "a");
	dup_test("bb",     "bb");

	/* test     expected arg1  arg2 */
	concat_test("",      "",   "");
	concat_test("a",     "a",  "");
	concat_test("b",     "",   "b");
	concat_test("ab",    "a",  "b");
	concat_test("ABCD",  "AB", "CD");

	/* test   expected   args... */
	join_test("",        "");
	join_test("a",       "a");
	join_test("",        "",  "");
	join_test("a",       "a", "");
	join_test("ab",      "a", "b");
	join_test("",        "",  "",  "");
	join_test("a",       "a", "",  "");
	join_test("b",       "",  "b", "");
	join_test("ab",      "a", "b", "");
	join_test("c",       "",  "",  "c");
	join_test("ac",      "a", "",  "c");
	join_test("bc",      "",  "b", "c");
	join_test("abc",     "a", "b", "c");
	join_test("abcdefg", "a", "b", "c", "", "", "d", "e", "", "", "fg");

	return 0;
}

