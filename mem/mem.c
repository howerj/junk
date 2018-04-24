/**@brief Utility for reading Kernel memory
 * @todo Add modified S-Expression parser library from gladiator program for
 * configuration
 * @todo Optional colorization of output
 * @todo Non-interactive mode
 * @todo Decoding functions?
 * @todo Add license
 * @todo Set output base, add commands for setting the verbosity and other
 * options at run time.
 * @todo Replace read with mmap, which should be more robust.
 * @todo Generate documentation/man pages from readme.md
 *
 * https://bbs.archlinux.org/viewtopic.php?id=121964
 * https://lwn.net/Articles/147901/
 * https://forum.kernelnewbies.org/read.php?13,2316
 * https://stackoverflow.com/questions/30680152/what-is-real-use-of-dev-kmem-char-device-in-linux
 *
 * */
#include "libline.h"
#include "mpc.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_HIST_LINES (100u)

typedef enum {
	LOG_LEVEL_ALL_OFF,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_NOTE,
	LOG_LEVEL_DEBUG, /* make an option to run a monitor if in this mode when _logger encountered? */
	LOG_LEVEL_ALL_ON
} log_level_e;

static const char *histfile = "reader.log";
static bool hist_enabled = true;
static const char *ifile = "/dev/mem";
static const char *prompt = "> ";
static log_level_e log_level = LOG_LEVEL_WARNING;
static unsigned line_editor_mode = 0; /* normal mode, emacs mode, set to 1 for vi */
static bool die_on_warnings = 0;
static unsigned default_size = sizeof(default_size);

static const char *emsg(void)
{
	static const char *unknown = "unknown reason";
	const char *r = errno ? strerror(errno) : unknown;
	if(!r) 
		r = unknown;
	return r;
}

#define error(FMT, ...)   _logger(true,            LOG_LEVEL_ERROR,   __FILE__, __func__, __LINE__, FMT, ##__VA_ARGS__)
#define warning(FMT, ...) _logger(die_on_warnings, LOG_LEVEL_WARNING, __FILE__, __func__, __LINE__, FMT, ##__VA_ARGS__)
#define note(FMT, ...)    _logger(false,           LOG_LEVEL_NOTE,    __FILE__, __func__, __LINE__, FMT, ##__VA_ARGS__)
#define debug(FMT, ...)   _logger(false,           LOG_LEVEL_DEBUG,   __FILE__, __func__, __LINE__, FMT, ##__VA_ARGS__)

int _logger(bool die, log_level_e level, const char *file, const char *func, unsigned line, const char *fmt, ...)
{
	int r = 0;
	va_list args;
	if(level > log_level) 
		goto end;
	if((r = fprintf(stderr, "[%s %s %u] ", file, func, line)) < 0)
		goto end;
	va_start(args, fmt);
	if((r = vfprintf(stderr, fmt, args)) < 0)
		goto cleanup;
	if((r = fputc('\n', stderr)) != '\n')
		goto cleanup;
cleanup:
	va_end(args);
end:
	if(die)
		exit(EXIT_FAILURE);
	return r;
}

bool verbose(log_level_e level)
{
	return level <= log_level && log_level != LOG_LEVEL_ALL_OFF;
}

int numberify(int base, long long *n, const char *s)
{
	assert(!base || (base > 1 && base < 37));
	assert(n && s);
	char *end = NULL;
	errno = 0;
	*n = strtoll(s, &end, base);
	return !errno && *s != '\0' && *end == '\0';
}

void *allocate(size_t sz)
{
	errno = 0;
	void *r = calloc(sz, 1);
	if(!r)
		error("allocate failed: %s", emsg());
	return r;
}

void *reallocate(void *p, size_t n)
{
	void *r = realloc(p, n); 
	if(!r)
		error("reallocate failed: %s", emsg());
	return r;
}

char *duplicate(const char *s)
{
	assert(s);
	size_t length = strlen(s) + 1;
	char *r = allocate(length);
	memcpy(r, s, length);
	return r;
}

char *slurp(FILE *f)
{ /**@warning does not work for large file >4GB */ 
	long length = 0, r = 0;
	char *b = NULL;
	errno = 0;
	if((r = fseek(f, 0, SEEK_END)) < 0)
		goto fail;
	errno = 0;
	if((r = length = ftell(f)) < 0)
		goto fail;
	errno = 0;
	if((r = fseek(f, 0, SEEK_SET)) < 0)
		goto fail;
	b = allocate(length + 1);
	errno = 0;
	if((unsigned long)length != fread(b, 1, length, f))
		goto fail;
	return b;
fail:
	free(b);
	warning("slurp failed: %s", emsg());
	return NULL;
}

#define X_MACRO_PARSE_VARS\
	X(sign,         "sign")\
	X(floating,     "float")\
	X(octal,        "octal")\
	X(decimal,      "decimal")\
	X(hex,          "hex")\
	X(unsignedn,    "unsigned")\
	X(signedn,      "signed")\
	X(number,       "number")\
	X(asfloat,      "asfloat")\
	X(reading,      "read")\
	X(writing,      "write")\
	X(command_line, "line")

enum cleanup_length_e
{
#define X(CVAR, NAME) _ignore_me_ ## CVAR,
	X_MACRO_PARSE_VARS
	CLEANUP_LENGTH
#undef X
};

static const char *command_grammar = 
" sign      : '+' | '-' ;\n"
" float     : /[-+]?[0-9]+(\\.[0-9]+)?([eE][-+]?[0-9]+)?[fF]/ ;\n"
" octal     : /0[0-7]*/ ;\n"
" decimal   : /[1-9][1-9]*/ ;\n"
" hex       : /0[xX][0-9a-fA-F]+/ ;\n"
" unsigned  : <hex> | <octal> | <decimal> ;\n"
" signed    : <sign> <unsigned> ;\n"
" number    : <signed> | <unsigned> ; \n"
" asfloat   : 'f' | 'F' ;\n"
" length    : <unsigned> ;\n"
" read      : 'r' <number> ( <asfloat> | <length> )? ;\n"
" write     : 'w' <number> ( <float> | <number> ) <length>? ;\n"
" line      : <read> | <write> ; \n";

static mpc_ast_t *_parse_command_string(const char *file_name, const char *string)
{
	#define X(CVAR, NAME) mpc_parser_t *CVAR = mpc_new((NAME));
	X_MACRO_PARSE_VARS
	#undef X

	#define X(CVAR, NAME) CVAR,
	mpc_err_t *language_error = mpca_lang(MPCA_LANG_DEFAULT, command_grammar, X_MACRO_PARSE_VARS NULL);
	#undef X

	if (language_error != NULL) {
		mpc_err_print(language_error);
		mpc_err_delete(language_error);
		exit(EXIT_FAILURE);
	}

	mpc_result_t r;
	mpc_ast_t *ast = NULL;
	if (mpc_parse(file_name, string, command_line, &r)) {
		ast = r.output;
	} else {
		mpc_err_print(r.error);
		mpc_err_delete(r.error);
	}

#define X(CVAR, NAME) CVAR,
	mpc_cleanup(CLEANUP_LENGTH,
		X_MACRO_PARSE_VARS NULL);
#undef X
	return ast;
}

static mpc_ast_t *parse_command_string(const char *string)
{
	return _parse_command_string(ifile, string);
}

typedef struct {
	union {
		unsigned long long ull;
		long long ll;
		float flt;
		double dbl;
	} value;
	long long addr;
	unsigned size;
	enum {
		AS_UNSIGNED,
		AS_SIGNED,
		AS_FLOAT,
		AS_DOUBLE
	} as;
	enum {
		MODE_READ,
		MODE_WRITE
	} mode;
	bool valid_command;
} command_t;

command_t analyze(mpc_ast_t *ast)
{
	assert(ast);
	int r = 0;
	command_t cmd   = {
		.value.ull = 0uLL,
		.mode      = MODE_READ,
		.size      = default_size,
		.addr      = 0,
		.as        = AS_UNSIGNED,
		.valid_command = true
	};
	mpc_ast_t *mode = mpc_ast_get_child(ast, "read|>");
	if(!mode) {
		mode = mpc_ast_get_child(ast, "write|>");
		assert(mode);
		cmd.mode = MODE_WRITE;
		char *number = mode->children[2]->contents;
		r = numberify(0, &cmd.value.ll, number);
		if(!r) {
			r = sscanf(number, "%lf", &cmd.value.dbl);
		}
		assert(r);
	}
	r = numberify(0, &cmd.addr, mode->children[1]->contents);
	assert(r);

	return cmd;
}

int command(int fd, const char *line, FILE *output)
{
	assert(fd != -1 && line && output);
	uint8_t *m = NULL;

	mpc_ast_t *ast = parse_command_string(line);
	if(!ast)
		goto fail;
	if(verbose(LOG_LEVEL_NOTE))
		mpc_ast_print(ast);
	command_t cmd = analyze(ast);
	m = allocate(cmd.size);

	errno = 0;
	if(lseek(fd, cmd.addr, SEEK_SET) == -1) {
		warning("seek failed: %s", emsg());
		goto fail;
	}

	if(read(fd, m, cmd.size) == -1) {
		warning("read failed: %s", emsg());
		goto fail;
	}

	for(unsigned i = 0; i < cmd.size; i++) {
		if(fprintf(output, "%02x ", m[i]) < 0) {
			warning("failed to write to output file");
			goto fail;
		}
	}

	if(fputc('\n', output) != '\n') {
		warning("failed to write to output file");
		goto fail;
	}

	free(m);
	return 0;
fail:
	free(m);
	debug("command failed");
	return -1;
}

static void print_usage(const char *prog)
{
	fprintf(stderr, "usage: %s [-hvHViD]\n", prog);
	fputs("  -h --help     print out this help message and exit\n"
	      "  -v --verbose  increase verbosity level\n"
	      "  -H --history  set the history file\n"
	      "  -V --vi       set line editor to vi mode\n"
	      "  -i --input    set input file\n"
	      "  -D --die      die if a warning occurs\n"
	      , stderr);
	exit(EXIT_FAILURE);
}

static void parse_opts(int argc, char *argv[])
{
	static const struct option lopts[] = {
		{ "help",    0, 0, 'h' },
		{ "verbose", 0, 0, 'v' },
		{ "vi",      0, 0, 'V' },
		{ "history", 1, 0, 'H' },
		{ "die",     0, 0, 'D' },
		{ "input",   1, 0, 'i' },
		{ NULL,      0, 0, 0 },
	};
	int c;

	while ((c = getopt_long(argc, argv, "hvVH:Di:", lopts, NULL)) != -1) {
		switch (c) {
		case 'v':
			log_level = log_level > LOG_LEVEL_ALL_ON ? log_level : log_level + 1;
			break;
		case 'V':
			line_editor_mode = 1;
			break;
		case 'H':
			histfile = optarg;
			break;
		case 'D':
			die_on_warnings = true;
			break;
		case 'i':
			ifile = optarg;
			break;
		default: 
			warning("invalid option %c", c);
			/*fall through*/
		case 'h':
			print_usage(argv[0]);
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	char *line = NULL;
	int fd = -1;

	parse_opts(argc, argv);
	line_set_vi_mode(!!line_editor_mode);

	note("mode      = %s", line_editor_mode ? "vi" : "emacs");
	note("verbosity = %d", log_level); /* yes, this is intentional */


	if(hist_enabled) {
		/* fails if file does not exist, for example on the first run */
		if(line_history_load(histfile) < 0)
			note("%s failed to load: %s", histfile, emsg());

		if(line_history_set_maxlen(MAX_HIST_LINES) < 0) {
			warning("could not set history file size to %u", MAX_HIST_LINES);
			hist_enabled = false;
		}
	}

	errno = 0;
	fd = open(ifile, O_RDONLY);
	if(fd == -1)
		error("could not open %s for reading: %s", ifile, emsg());

	while((line = line_editor(prompt))) {
		debug("%s", line);
		if(command(fd, line, stdout) < 0) 
			warning("invalid command");

		if(hist_enabled && line_history_add(line) < 0) {
			warning("could not add line to history, disabling history feature");
			hist_enabled = false;
		}
		free(line);
	}

	errno = 0;
	if(close(fd) == -1)
		warning("could not close file handle: %s", emsg());

	errno = 0;
	if(hist_enabled && line_history_save(histfile) < 0)
		warning("could not save history to %s: %s", histfile, emsg());

	return 0;
}

