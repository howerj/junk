/* rjh 2019 */
#include <stdio.h>
#include <string.h>
#include <assert.h>

static int match(const char *s) {
	assert(s);
	char st[512]; // = { 0 };
	int sp = 0;
	for (size_t i = 0; s[i]; i++) {
		switch (s[i]) {
		case '(': assert(sp < sizeof(st)); st[sp++] = '('; break;;
		case '[': assert(sp < sizeof(st)); st[sp++] = '['; break;;
		case '{': assert(sp < sizeof(st)); st[sp++] = '{'; break;;
		case ')': if (!sp || st[--sp] != '(') return 0; break;;
		case ']': if (!sp || st[--sp] != '[') return 0; break;;
		case '}': if (!sp || st[--sp] != '{') return 0; break;;
		default:
			  return -1;
		}
	}
	return sp == 0;
}

static int tests(FILE *out) {
	assert(out);
	static const struct tests {
		int result;
		const char *test;
	} ts[] = {
		{  1, ""  },
		{  0, "(" },
		{  0, "]" },
		{  1, "()" },
		{  1, "[]" },
		{  1, "{}" },
		{  1, "{}()" },
		{ -1, "{} ()" },
		{  1, "{}()" },
		{  1, "{()}" },
		{  1, "{()[]}" },
		{  0, "{]()[]}" },
		{  0, "{()][]}" },
		{  0, "{()][]}" },
		{  1, "(([{}][]){})" },
	};
	int r = 0;
	const size_t l = sizeof ts / sizeof ts[0];
	for (size_t i = 0; i < l; i++) {
		const int result = match(ts[i].test);
		if (result != ts[i].result) {
			r = 1;
			fputs("FAIL: ", out);
		} else {
			fputs("ok:   ", out);
		}
		fprintf(out, "'%s' => %d\n", ts[i].test, ts[i].result);

	}
	return r;
}


int main(int argc, char **argv) {
	if (argc == 1) {
		printf("running self tests\n");
		return tests(stdout);
	}

	if (argc != 2) {
		fprintf(stderr, "usage: %s parens\n", argv[0]);
		return 1;
	}
	const int m = match(argv[1]);
	if (m < 0)
		printf("invalid input string: %s\n", argv[1]);
	else if (m == 0)
		printf("no match: %s\n", argv[1]);
	else
		printf("match: %s\n", argv[1]);
	return 0;
}

