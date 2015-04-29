#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void util_fatal(char *msg, char *file, unsigned line)
{
        util_warn(msg, file, line);
        exit(EXIT_FAILURE);
}

void util_warn(char *msg, char *file, unsigned line)
{
        assert(msg && file);
        fprintf(stderr, "(%s %s %u)\n", msg, file, line);
}

