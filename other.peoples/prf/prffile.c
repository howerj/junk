/* Written by Brian Raiter, January 2001. Placed in the public domain. */

#include	<stdio.h>

#define fail(msg)	(printf("%s\n", (msg)), 1)

static int prffile(FILE *fp, char const *filename)
{
    unsigned char	buf[4];
    int			width, height, bits, planes;
    char		fmt;

    printf("%s:\t", filename);

    if (fread(buf, 4, 1, fp) != 1)
	return fail("missing signature");

    fmt = buf[0];
    if ((buf[0] != 'P' && buf[0] != 'M') || buf[1] != 'R'
					 || buf[2] != 'F'
					 || buf[3] != '1')
	return fail("invalid signature");

    if (fread(buf, 4, 1, fp) != 1)
	return fail("incomplete header");
    width = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
    if (width <= 0)
	return fail("invalid width");

    if (fread(buf, 4, 1, fp) != 1)
	return fail("incomplete header");
    height = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
    if (height <= 0)
	return fail("invalid height");

    if (fread(buf, 1, 1, fp) != 1)
	return fail("incomplete header");
    if (fmt == 'M' && buf[0])
	return fail("invalid mrf header");
    bits = (buf[0] & 31) + 1;
    planes = ((buf[0] >> 5) & 7) + 1;

    printf("%cRF, %d by %d", fmt, width, height);
    if (fmt == 'P')
	printf(", %d x %d bits/pixel", bits, planes);
    putchar('\n');

    return 0;
}

int main(int argc, char *argv[])
{
    FILE       *fp;
    int		i;

    if (argc < 2)
	return prffile(stdin, "stdin");

    for (i = 1 ; i < argc ; ++i) {
	if (!(fp = fopen(argv[i], "r"))) {
	    perror(argv[i]);
	    return 1;
	}
	if (prffile(fp, argv[i]))
	    return 1;
	fclose(fp);
    }

    return 0;
}
