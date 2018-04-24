/** 
@file     unit.c
@brief    unit tests for libcompress public interface
@author   Richard Howe
@license  MIT (see https://opensource.org/licenses/MIT)
@email    howe.r.j.89@gmail.com 
**/  

/*** module to test ***/
#include "libcompress.h"
/**********************/

#include <assert.h>
#include <inttypes.h>
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*** very minimal test framework ***/

typedef struct {
	unsigned passed, failed; /** number of tests passed and failed */
	double timer;
	clock_t start_time, end_time;
	time_t rawtime;
	int color_on;      /**< Is colorized output on?*/
	int jmpbuf_active; /**< Have we setup the longjmp buffer or not? */
	int is_silent;     /**< Silent mode on? The programs return code is used to determine success*/
	jmp_buf current_test; /**< current unit tests jump buffer */
	unsigned current_line; /**< current line number of unit test being executed */
	int current_result;    /**< result of latest test execution */
	const char *current_expr;  /**< string representation of expression being executed */
	FILE *output;
	int caught_signal; /**@warning signal catching is supported, but not thread safe */
} test_t;

static test_t tb;

#define MAX_SIGNALS (256) /**< maximum number of signals to decode */
static char *(sig_lookup[]) = { /*List of C89 signals and their names*/
	[SIGABRT]       = "SIGABRT",
	[SIGFPE]        = "SIGFPE",
	[SIGILL]        = "SIGILL",
	[SIGINT]        = "SIGINT",
	[SIGSEGV]       = "SIGSEGV",
	[SIGTERM]       = "SIGTERM",
	[MAX_SIGNALS]   = NULL
};

/*
static size_t compare(const char *restrict a, const char *restrict b, size_t c)
{
	for(size_t i = 0; i < c; i++)
		if(a[i] != b[i])
			return i;
	return 0;
}*/

static void print_caught_signal_name(test_t *t) 
{
	char *sig_name = "UNKNOWN SIGNAL";
	if((t->caught_signal > 0) && (t->caught_signal < MAX_SIGNALS) && sig_lookup[t->caught_signal])
		sig_name = sig_lookup[t->caught_signal];
	if(!(t->is_silent))
		fprintf(t->output, "caught %s (signal number %d)\n", sig_name, t->caught_signal);\
}

/**@warning not thread-safe, this function uses internal static state*/
static void sig_abrt_handler(int sig) 
{ /* catches assert() from within functions being exercised */
	tb.caught_signal = sig;
	if(tb.jmpbuf_active) {
		tb.jmpbuf_active = 0;
		longjmp(tb.current_test, 1);
	}
}

static const char *reset(test_t *t)  { return t->color_on ? "\x1b[0m"  : ""; }
static const char *red(test_t *t)    { return t->color_on ? "\x1b[31m" : ""; }
static const char *green(test_t *t)  { return t->color_on ? "\x1b[32m" : ""; }
static const char *yellow(test_t *t) { return t->color_on ? "\x1b[33m" : ""; }
static const char *blue(test_t *t)   { return t->color_on ? "\x1b[34m" : ""; }

static int unit_tester(test_t *t, const int test, const char *msg, unsigned line) 
{
	assert(t && msg);
	if(test) {
	       	t->passed++; 
		if(!(t->is_silent))
			fprintf(t->output, "      %sok%s:\t%s\n", green(t), reset(t), msg); 
	} else { 
		t->failed++;
	       	if(!(t->is_silent))
			fprintf(t->output, "  %sFAILED%s:\t%s (line %d)\n", red(t), reset(t), msg, line);
	}
	return test;
}

static void print_statement(test_t *t, const char *stmt) 
{
	assert(t);
	if(!(t->is_silent))
		fprintf(t->output, "   %sstate%s:\t%s\n", blue(t), reset(t), stmt);
}

static void print_must(test_t *t, const char *must) 
{
	assert(t);
	if(!(t->is_silent))
		fprintf(t->output, "    %smust%s:\t%s\n", blue(t), reset(t), must);
}

static void print_note(test_t *t, const char *name) 
{ 
	assert(t);
	if(!(t->is_silent))
		fprintf(t->output, "%s%s%s\n", yellow(t), name, reset(t)); 
}

/**@brief Advance the test suite by testing and executing an expression. This
 *        framework can catch assertions that have failed within the expression
 *        being tested.
 * @param EXPR The expression should yield non zero on success **/
#define test(TESTBENCH, EXPR) _test((TESTBENCH), (EXPR) != 0, #EXPR, __LINE__)

static void _test(test_t *t, const int result, const char *expr, const unsigned line)
{
	assert(t && expr);
	t->current_line = line,
	t->current_expr = expr;
	signal(SIGABRT, sig_abrt_handler);
	if(!setjmp(t->current_test)) {
		t->jmpbuf_active = 1;
		t->current_result = unit_tester(t, result, t->current_expr, t->current_line);
	} else {
		print_caught_signal_name(t);
		t->current_result = unit_tester(t, 0, t->current_expr, t->current_line);
		signal(SIGABRT, sig_abrt_handler);
	}
	signal(SIGABRT, SIG_DFL);
	t->jmpbuf_active = 0;
}

/**@brief This advances the test suite like the test macro, however this test
 * must be executed otherwise the test suite will not continue
 * @param EXPR The expression should yield non zero on success */
#define must(TESTBENCH, EXPR) _must((TESTBENCH), (EXPR) != 0, #EXPR, __LINE__)

static void _must(test_t *t, const int result, const char *expr, const unsigned line)
{
	assert(t && expr);
	print_must(t, expr);
	_test(t, result, expr, line);
	if(!(t->current_result))
		exit(-1);
}

/**@brief print out and execute a statement that is needed to further a test
 * @param STMT A statement to print out (stringify first) and then execute**/
#define state(TESTBENCH, STMT) do{ print_statement((TESTBENCH), #STMT ); STMT; } while(0);

static int unit_test_start(test_t *t, const char *unit_name, FILE *output) 
{
	assert(t && unit_name && output);
	memset(t, 0, sizeof(*t));
	time(&t->rawtime);
	t->output = output;
	if(signal(SIGABRT, sig_abrt_handler) == SIG_ERR) {
		fprintf(stderr, "signal handler installation failed");
		return -1;
	}
	t->start_time = clock();
	if(!(t->is_silent))
		fprintf(t->output, "%s unit tests\n%sbegin:\n\n", 
				unit_name, asctime(localtime(&(t->rawtime))));
	return 0;
}

static unsigned unit_test_end(test_t *t, const char *unit_name) 
{
	assert(t && unit_name);
	t->end_time = clock();
	t->timer = ((double) (t->end_time - t->start_time)) / CLOCKS_PER_SEC;
	if(!(t->is_silent))
		fprintf(t->output, "\n\n%s unit tests\npassed  %u/%u\ntime    %fs\n", 
				unit_name, t->passed, t->passed+t->failed, t->timer);
	return t->failed;
}

/*** end minimal test framework ***/

int libcompress_unit_tests(FILE *output, int colorize, int silent)
{
	tb.is_silent = silent;
	tb.color_on  = colorize;

	unit_test_start(&tb, "libcompress", output);
	{
		uint8_t buf[128] = {0};
		io_t *o = NULL;
		print_note(&tb, "Testing IO functions");
		must(&tb, o = io_string_external(IO_RW, sizeof(buf), buf));
		test(&tb, 'c' == io_putc('c', o));
		test(&tb, '\n' == io_putc('\n', o));
		test(&tb, 'c' == io_getc(o));
		test(&tb, '\n' == io_getc(o));
		test(&tb, 2 == io_get_chars_read(o));
		test(&tb, 2 == io_get_chars_written(o));
		test(&tb, io_get_chars_written(o) == io_get_chars_read(o));
		state(&tb, io_free(o));
	}
	return !!unit_test_end(&tb, "libcompress");
}

int main(void)
{
	return libcompress_unit_tests(stdout, 0, 0);
}

