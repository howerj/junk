/**@author Richard James Howe
 * @license MIT */
#include "system.h"

static inline int extract(unsigned char c, int radix)
{
	c = tolower(c);
	if(c >= '0' && c <= '9')
		c -= '0';
	else if(c >= 'a' && c <= 'z')
		c -= ('a' - 10);
	else
		return -1;
	if(c < radix)
		return c;
	return -1;
}

#ifndef MIN
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif

#ifndef MAX
#define MAX(X, Y) ((X) < (Y) ? (Y) : (X))
#endif

/** @brief This function should behave the same as the standard C library
 * function 'strtol', except the string length is limited by 'length'. It
 * will attempt to convert the provided string.
 *  @param [in] str, A NUL terminated ASCII string, possibly representing a
 *  number. An optional '+' may be prefixed to the number, and a '-' can
 *  be used to indicate negative numbers
 *  @param [in] length, maximum length of the string, regardless of any
 *  NUL terminator (or lack thereof).
 *  @param [out] endptr, pointer to last unconverted digit, *or* a pointer
 *  to the last character if 'length' would be exceeded. (@note would
 *  it be better to point after?)
 *  @param [in] base, The conversion radix. Valid values range from 2-36 with
 *  the characters 'A'-'Z' (or 'a'-'z') representing numbers from 10-35 once 
 *  the decimal  digits run out. '0' is a special case that allows a 
 *  prefix (coming after any optional '+' or '-') to indicate whether the 
 *  string is in octal (prefix '0'), hexadecimal (prefix "0x" or "0X") or decimal
 *  (no prefix).
 *  @return The converted value is returned, or zero on failure, as
 *  zero is a valid number an analysis of the 'endptr' is needed to
 *  determine if the conversion succeeded. If the result is out of range
 *  'LONG_MAX' is returned for positive numbers an 'LONG_MIN' is returned for
 *  negative numbers and 'errno' is set to 'ERANGE'
 */
long int strntol(const char *str, size_t length, const char **endptr, int base)
{
	assert(str);
	assert(endptr);
	assert((base >= 2 && base <= 36) || !base);

	size_t i = 0;
	bool negate = false;
	bool overflow = false;
	unsigned radix = base;
	unsigned long r = 0;

	for(; i < length && str[i]; i++) /* ignore leading white space */
		if(!isspace(str[i]))
			break;
	if(i >= length)
		goto end;

	if(str[i] == '-' || str[i] == '+') { /* Optional '+', and '-' for negative */
		if(str[i] == '-')
			negate = true;
		i++;
	}
	if(i >= length)
		goto end;

	radix = base;
	/* 0 is a valid number, but is '0x'? */
	if(!base || base == 16) { /* prefix 0 = octal, 0x or 0X = hex */
		if(str[i] == '0') {
			if(((i+1) < length) && (str[i + 1] == 'x' || str[i + 1] == 'X')) {
				radix = 16;
				i += 2;
			} else {
				radix = 8;
				i++;
			}
		} else {
			if(!base)
				radix = 10;
		}
	}
	if(i >= length)
		goto end;

	for(; i < length && str[i]; i++) {
		int e = extract(str[i], radix);
		if(e < 0)
			break;
		unsigned long a = e;
		r = (r*radix) + a;
		if(r > LONG_MAX) /* continue on with conversion */
			overflow = true;
	}

end:
	i = MIN(i, length);
	*endptr = &str[i];
	if(overflow) { /* result out of range, set errno to indicate this */
		errno = ERANGE;
		r = LONG_MAX;
		if(negate)
			return LONG_MIN;
	}
	if(negate)
		r = -r;
	return r;
}

static int test(FILE *out, const char *s, size_t length, int base, long expected)
{
	assert(out);
	assert(s);
	const char *end = NULL;
	bool pass = false;
	fprintf(out, "test:   string = \"%s\", max = %zu, base = %d\n", s, length, base);
	errno = 0;
	long r = strntol(s, length, &end, base);
	fprintf(out, "result: r = %ld, errno = %d, end-s = %zu\n", r, errno, (size_t)(end-s));

	pass = expected == r;
	fprintf(out, "%s:  %ld %s %ld\n", pass ? "    ok" : "FAILED", expected, pass ? "==" : "<>", r);
	fputc('\n', out);
	return r == expected;
}

int main(void)
{
	FILE *out = stdout;
	fprintf(out, "ERANGE = %d, LONG_MIN = %lu, LONG_MAX = %lu\n\n", ERANGE, LONG_MIN, LONG_MAX);
	unsigned passed = 0, total = 0;

	typedef struct {
		int base;
		size_t length;
		long expected;
		char *s;
	} test_t;
	static const test_t tests[] = { /** @todo add expected 'errno' and 'endptr' */
		{  10,  255,  0,       ""        },
		{  10,  255,  0,       "0"       },
		{  10,  255,  0,       "0x"      },
		{  16,  255,  0,       "0X"      },
		{  16,  255,  0,       "0x"      },
		{  16,  255,  0xABCD,  "0xABCD"  },
		{  16,  255,  0xABCD,  "ABCD"    },
		{  16,  255,  0xABCD,  "abcd"    },
		{  16,  255,  0x12CD,  "12cd"    },
		{  16,  255,  0xABCD,  "AbCd"    },
		{  10,  255,  1234,    "1234"    },
		{  10,  2,    12,      "1234"    },
		{  10,  0,    0,       "1234"    },
		{  10,  255,  -1234,   "   -1234"   },
		{  10,  255,  -1234,   "-1234"   },
		{  10,  255,  1234,    "   1234"    },
		{  0,   255,  0x1234,  "0x1234"  },
		{  0,   255,  01234,   "01234"  },
		{  0,   255,  -01234,  "-01234"  },
		{  10,   255,  1234,   "01234"  },
		{  0,   255,  1,       "1"  },
		{  2,   255,  5,       "1012"  },
		{  0,   255,  LONG_MAX,  "123456789123456789123456789"  },
		{  0,   255,  LONG_MIN,  "-123456789123456789123456789"  },
	};

	total = sizeof(tests)/sizeof(tests[0]);
	for(size_t i = 0; i < total; i++)
		passed += test(out, tests[i].s, tests[i].length, tests[i].base, tests[i].expected);

	fprintf(out, "passed/total: %u/%u\n", passed, total);
	fprintf(out, "finished: %s\n\n", passed == total ? "ALL SUCCEEDED" : "TEST SUITE FAILE");

	return !(passed == total);
}
