/* Written by Brian Raiter, January 2001. Placed in the public domain. */

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	"gen.h"
#include	"pnm.h"

/*
 * pbm functions
 */

static int writepbmheader(fileinfo *file, imageinfo *image)
{
    if (fprintf(file->fp, "P4\n%d %d\n", image->width, image->height) <= 0)
	return fileerr(file);
    return TRUE;
}

static int writepbmrawrow(fileinfo *file, imageinfo *image)
{
    uchar      *line;
    uchar	byte, b;
    int		y, x, w;

    y = image->height - image->ypos;
    if (y <= 0)
	return -1;
    if (y > squaresize)
	y = squaresize;
    for (line = image->buf[0] ; y ; --y, line += image->width) {
	for (x = 0 ; x < image->width ; x += 8) {
	    byte = 0;
	    b = 128;
	    for (w = 0 ; w < 8 && x + w < image->width ; ++w, b >>= 1)
		if (!line[x + w])
		    byte |= b;
	    if (fputc(byte, file->fp) == EOF)
		return fileerr(file);
	}
    }
    return TRUE;
}

/*
 * pgm functions
 */

static int writepgmheader(fileinfo *file, imageinfo *image)
{
    if (fprintf(file->fp, "P5\n%d %d\n%d\n",
		image->width, image->height, (1 << image->bits) - 1) <= 0)
	return fileerr(file);
    return TRUE;
}

static int writepgmrawrow(fileinfo *file, imageinfo *image)
{
    int	y;

    y = image->height - image->ypos;
    if (y <= 0)
	return -1;
    if (y > squaresize)
	y = squaresize;
    if ((int)fwrite(image->buf[0], image->width, y, file->fp) != y)
	return fileerr(file);
    return TRUE;
}

/*
 * ppm functions
 */

static int writeppmheader(fileinfo *file, imageinfo *image)
{
    if (fprintf(file->fp, "P6\n%d %d\n%d\n",
		image->width, image->height, (1 << image->bits) - 1) <= 0)
	return fileerr(file);
    return TRUE;
}

static int writeppmrawrow(fileinfo *file, imageinfo *image)
{
    uchar      *rline, *gline, *bline;
    int		y, x;

    y = image->height - image->ypos;
    if (y <= 0)
	return -1;
    if (y > squaresize)
	y = squaresize;
    rline = image->buf[0];
    gline = image->buf[1];
    bline = image->buf[2];
    while (y--) {
	for (x = 0 ; x < image->width ; ++x)
	    if (fputc(rline[x], file->fp) == EOF
				|| fputc(gline[x], file->fp) == EOF
				|| fputc(bline[x], file->fp) == EOF)
		return fileerr(file);
	rline += image->width;
	gline += image->width;
	bline += image->width;
    }
    return TRUE;
}

/*
 * exported functions
 */

int writepnmheader(fileinfo *file, imageinfo *image)
{
    switch (file->type) {
      case pbmraw:  return writepbmheader(file, image);
      case pgmraw:  return writepgmheader(file, image);
      case ppmraw:  return writeppmheader(file, image);
    }
    return FALSE;
}

int writepnmrow(fileinfo *file, imageinfo *image)
{
    switch (file->type) {
      case pbmraw:  return writepbmrawrow(file, image);
      case pgmraw:  return writepgmrawrow(file, image);
      case ppmraw:  return writeppmrawrow(file, image);
    }
    return FALSE;
}
