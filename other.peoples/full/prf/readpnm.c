/* Written by Brian Raiter, January 2001. Placed in the public domain. */

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include        <string.h>
#include	"gen.h"
#include	"pnm.h"

static int getline(fileinfo *file, char *buf, int len)
{
    int	ch, n;

    do {
	if (!fgets(buf, len, file->fp))
	    return -1;
	n = strlen(buf);
	if (n == len - 1 && buf[n] != '\n') {
	    do
		ch = fgetc(file->fp);
	    while (ch != EOF && ch != '\n');
	} else if (buf[n - 1] == '\n')
	    buf[--n] = '\0';
    } while (!n || *buf == '#');
    return n;
}

/*
 * pbm functions
 */

static int readpbmheader(fileinfo *file, imageinfo *image)
{
    char	buf[256];

    if (getline(file, buf, sizeof buf - 1) <= 0)
	return err("invalid pbm header");
    if (sscanf(buf, "%d %d", &image->width, &image->height) != 2)
	return err("invalid pbm size data");
    if (image->width <= 0 || image->height <= 0)
	return err("invalid pbm image size");

    image->bits = 1;
    image->planes = 1;
    if (!(image->buf[0] = calloc(image->width, squaresize)))
	return err(NULL);
    image->ypos = 0;

    return TRUE;
}

static int readpbmrawrow(fileinfo *file, imageinfo *image)
{
    uchar      *line;
    int		byte;
    int		y, x, w;

    y = image->height - image->ypos;
    if (y <= 0)
	return 0;
    if (y > squaresize)
	y = squaresize;
    for (line = image->buf[0] ; y ; --y, line += image->width) {
	for (x = 0 ; x < image->width ; x += 8) {
	    if ((byte = fgetc(file->fp)) == EOF) {
		fileerr(file);
		return -1;
	    }
	    byte ^= 255;
	    for (w = 0 ; w < 8 && x + w < image->width ; ++w, byte <<= 1)
		line[x + w] = byte & 128 ? 1 : 0;
	}
    }
    return 1;
}

static int readpbmascrow(fileinfo *file, imageinfo *image)
{
    uchar      *line;
    int		pixel;
    int		y, x;

    y = image->height - image->ypos;
    if (y <= 0)
	return 0;
    if (y > squaresize)
	y = squaresize;
    for (line = image->buf[0] ; y ; --y, line += image->width) {
	for (x = 0 ; x < image->width ; ++x) {
	    if (fscanf(file->fp, "%1d", &pixel) < 1) {
		fileerr(file);
		return -1;
	    }
	    if (pixel == '0')
		line[x] = 1;
	}
    }
    return 1;
}

/*
 * pgm functions
 */

static int readpgmheader(fileinfo *file, imageinfo *image)
{
    char	buf[256];
    int		n;

    if (getline(file, buf, sizeof buf - 1) <= 0)
	return err("invalid pgm header");
    if (sscanf(buf, "%d %d", &image->width, &image->height) != 2)
	return err("invalid pgm size data");
    if (image->width <= 0 || image->height <= 0)
	return err("invalid pgm image size");

    if (getline(file, buf, sizeof buf - 1) <= 0)
	return err("invalid pgm header");
    if (sscanf(buf, "%d", &n) != 1 || n < 1)
	return err("invalid pgm grayscale data");
    if (n >= 256)
	return err("too many shades of gray");
    for (image->bits = 0, ++n ; !(n & 1) ; ++image->bits, n >>= 1) ;
    if (n != 1)
	return err("grayscale must be a power of two");

    image->planes = 1;
    if (!(image->buf[0] = calloc(image->width, squaresize)))
	return err(NULL);
    image->ypos = 0;

    return TRUE;
}

static int readpgmrawrow(fileinfo *file, imageinfo *image)
{
    int	y;

    y = image->height - image->ypos;
    if (y <= 0)
	return 0;
    if (y > squaresize)
	y = squaresize;
    if ((int)fread(image->buf[0], image->width, y, file->fp) != y) {
	fileerr(file);
	return -1;
    }
    return 1;
}

static int readpgmascrow(fileinfo *file, imageinfo *image)
{
    uchar      *line;
    int		pixel;
    int		y, x;

    y = image->height - image->ypos;
    if (y <= 0)
	return 0;
    if (y > squaresize)
	y = squaresize;
    for (line = image->buf[0] ; y ; --y, line += image->width) {
	for (x = 0 ; x < image->width ; ++x) {
	    if (fscanf(file->fp, "%d", &pixel) < 1) {
		fileerr(file);
		return -1;
	    }
	    line[x] = pixel;
	}
    }
    return 1;
}

/*
 * ppm functions
 */

static int readppmheader(fileinfo *file, imageinfo *image)
{
    char	buf[256];
    int		n;

    if (getline(file, buf, sizeof buf - 1) <= 0)
	return err("invalid ppm header");
    if (sscanf(buf, "%d %d", &image->width, &image->height) != 2)
	return err("invalid ppm size data");
    if (image->width <= 0 || image->height <= 0)
	return err("invalid ppm image size");

    if (getline(file, buf, sizeof buf - 1) <= 0)
	return err("invalid ppm header");
    if (sscanf(buf, "%d", &n) != 1 || n < 1)
	return err("invalid ppm color data");
    if (n >= 256)
	return err("too many color levels");
    for (image->bits = 0, ++n ; !(n & 1) ; ++image->bits, n >>= 1) ;
    if (n != 1)
	return err("color scale must be a power of two");

    image->planes = 3;
    n = image->width * image->height;
    if (!(image->buf[0] = calloc(n, 3)))
	return err(NULL);
    image->buf[1] = image->buf[0] + n;
    image->buf[2] = image->buf[1] + n;
    image->ypos = 0;

    return TRUE;
}

static int readppmrawrow(fileinfo *file, imageinfo *image)
{
    uchar      *rline, *gline, *bline;
    int		r, g, b;
    int		y, x;

    y = image->height - image->ypos;
    if (y <= 0)
	return 0;
    if (y > squaresize)
	y = squaresize;
    rline = image->buf[0];
    gline = image->buf[1];
    bline = image->buf[2];
    while (y--) {
	for (x = 0 ; x < image->width ; ++x) {
	    if ((r = fgetc(file->fp)) == EOF || (g = fgetc(file->fp)) == EOF
					     || (b = fgetc(file->fp)) == EOF) {
		fileerr(file);
		return -1;
	    }
	    rline[x] = r;
	    gline[x] = g;
	    bline[x] = b;
	}
	rline += image->width;
	gline += image->width;
	bline += image->width;
    }
    return 1;
}

static int readppmascrow(fileinfo *file, imageinfo *image)
{
    uchar      *rline, *gline, *bline;
    int		r, g, b;
    int		y, x;

    y = image->height - image->ypos;
    if (y <= 0)
	return 0;
    if (y > squaresize)
	y = squaresize;
    rline = image->buf[0];
    gline = image->buf[1];
    bline = image->buf[2];
    while (y--) {
	for (x = 0 ; x < image->width ; ++x) {
	    if (fscanf(file->fp, "%d %d %d", &r, &g, &b) < 3) {
		fileerr(file);
		return -1;
	    }
	    rline[x] = r;
	    gline[x] = g;
	    bline[x] = b;
	}
	rline += image->width;
	gline += image->width;
	bline += image->width;
    }
    return 1;
}

/*
 * exported functions
 */

int readpnmheader(fileinfo *file, imageinfo *image)
{
    int	ch;

    if ((ch = fgetc(file->fp)) == EOF)
	return err("empty input file");
    if (ch != 'P')
	return err("invalid pnm signature");
    if ((file->type = fgetc(file->fp)) == EOF)
	return err("invalid input file");
    if (fgetc(file->fp) != '\n')
	return err("invalid pnm signature");
    switch (file->type) {
      case pbmasc: case pbmraw:  return readpbmheader(file, image);
      case pgmasc: case pgmraw:  return readpgmheader(file, image);
      case ppmasc: case ppmraw:  return readppmheader(file, image);
    }
    return err("invalid pnm signature");
}

int readpnmrow(fileinfo *file, imageinfo *image)
{
    switch (file->type) {
      case pbmasc:  return readpbmascrow(file, image);
      case pgmasc:  return readpgmascrow(file, image);
      case ppmasc:  return readppmascrow(file, image);
      case pbmraw:  return readpbmrawrow(file, image);
      case pgmraw:  return readpgmrawrow(file, image);
      case ppmraw:  return readppmrawrow(file, image);
    }
    return FALSE;
}
