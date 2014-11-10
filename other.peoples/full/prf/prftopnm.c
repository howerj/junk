/* Written by Brian Raiter, January 2001. Placed in the public domain. */

#include	<stdlib.h>
#include	<string.h>
#include	"gen.h"
#include	"prf.h"
#include	"pnm.h"

static void cmdline(int argc, char *argv[],
		    char const **arg1, char const **arg2)
{
    progname = argv[0];

    *arg1 = *arg2 = NULL;
    if (argc < 2)
	return;

    if (argv[1][0] == '-') {
	if (argv[1][1] == 'h' || !strcmp(argv[1], "--help")) {
	    puts("Usage: prftopnm [infile [outfile]]");
	    exit(EXIT_SUCCESS);
	} else if (argv[1][1] == 'v' || !strcmp(argv[1], "--version")) {
	    puts("prftopnm version 1.0, Copyright 2001 Brian Raiter, GPL.");
	    exit(EXIT_SUCCESS);
	}
    }

    *arg1 = argv[1];
    *arg2 = argv[2];
}

int main(int argc, char *argv[])
{
    fileinfo	in, out;
    imageinfo	image;
    int		f;

    cmdline(argc, argv, &in.name, &out.name);

    if (!in.name || !strcmp(in.name, "-"))
	f = openreadfile(&in, "stdin", stdin);
    else
	f = openreadfile(&in, in.name, NULL);
    if (!f)
	return EXIT_FAILURE;

    if (!out.name || !strcmp(out.name, "-"))
	f = openwritefile(&out, "stdout", stdout);
    else
	f = openwritefile(&out, out.name, NULL);
    if (!f)
	return EXIT_FAILURE;

    if (!readprfheader(&in, &image))
	return EXIT_FAILURE;
    out.type = image.planes > 1 ? ppmraw : image.bits > 1 ? pgmraw : pbmraw;
    if (!writepnmheader(&out, &image))
	return EXIT_FAILURE;
    while ((f = readprfrow(&in, &image))) {
	if (f < 0 || !writepnmrow(&out, &image))
	    return EXIT_FAILURE;
	image.ypos += squaresize;
    }

    closereadfile(&in);
    closewritefile(&out);
    return EXIT_SUCCESS;
}
