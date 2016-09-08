/* Written by Brian Raiter, January 2001. Placed in the public domain. */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	"gen.h"

char const      *progname = "error";

int err(char const *msg)
{
    if (msg)
	fprintf(stderr, "%s: %s\n", progname, msg);
    else if (errno)
	fprintf(stderr, "%s: %s\n", progname, strerror(errno));
    else
	fprintf(stderr, "%s: programmer error\n", progname);
    return FALSE;
}

int fileerr(fileinfo const *file)
{
    if (ferror(file->fp))
	fprintf(stderr, "%s: %s: %s\n", progname, file->name, strerror(errno));
    else if (feof(file->fp))
	fprintf(stderr, "%s: %s: unexpected EOF\n", progname, file->name);
    else
	fprintf(stderr, "%s: %s: programmer error\n", progname, file->name);
    return FALSE;
}

int openreadfile(fileinfo *file, char const *name, FILE *fp)
{
    file->name = name;
    if (!fp) {
	if (!(fp = fopen(name, "r"))) {
	    perror(name);
	    return FALSE;
	}
    }
    file->fp = fp;
    file->bitcount = 0;
    file->byte = 0;
    return TRUE;
}

int openwritefile(fileinfo *file, char const *name, FILE *fp)
{
    file->name = name;
    if (!fp) {
	if (!(fp = fopen(name, "w"))) {
	    perror(name);
	    return FALSE;
	}
    }
    file->fp = fp;
    file->bitcount = 0;
    file->byte = 0;
    return TRUE;
}

int closereadfile(fileinfo *file)
{
    if (fclose(file->fp)) {
	perror(file->name);
	return FALSE;
    }
    return TRUE;
}

int closewritefile(fileinfo *file)
{
    if (file->bitcount)
	if (fputc(file->byte << (8 - (file->bitcount & 7)), file->fp) == EOF)
	    return fileerr(file);

    if (fclose(file->fp)) {
	perror(file->name);
	return FALSE;
    }
    return TRUE;
}
