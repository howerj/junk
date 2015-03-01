/* Written by Brian Raiter, January 2001. Placed in the public domain. */

#include	<stdio.h>
#include	<stdlib.h>
#include        <string.h>
#include	"gen.h"
#include	"prf.h"

static fileinfo	       *file;
static imageinfo       *image;
static uchar	       *imagebuf;

/*
 * decoding functions
 */

static int bitsin(int nbits, uchar *val)
{
    int	bit;

    *val = 0;
    for (bit = 1 << (nbits - 1) ; bit ; bit >>= 1) {
	if (file->bitcount++ & 7)
	    file->byte <<= 1;
	else
	    if ((file->byte = fgetc(file->fp)) == EOF)
		return fileerr(file);
	if (file->byte & 0x80)
	    *val |= bit;
    }
    return TRUE;
}

static int decodesquare(int ypos, int xpos, int size, int cb, uchar pixel)
{
    static int const	bitsize[] = { 0, 1, 2, 2, 3, 3, 3, 3, 4 };
    uchar	       *square;
    uchar	       *sq;
    uchar		byte;
    uchar		count;
    int			y, n;

    if (file->type == prf)
	if (ypos >= image->height || xpos >= image->width)
	    return TRUE;

    square = imagebuf + (ypos % squaresize) * image->width + xpos;

    if (size == 1) {
	byte = 0;
	if (cb)
	    if (!bitsin(cb, &byte))
		return FALSE;
	*square = pixel | byte;
	return TRUE;
    }

    if (!bitsin(bitsize[cb], &count))
	return FALSE;
    cb -= count;
    if (count) {
	if (!bitsin(count, &byte))
	    return FALSE;
	pixel |= byte << cb;
    }

    if (!cb) {
	n = image->width - xpos;
	if (n > size)
	    n = size;
	y = image->height - ypos;
	if (y > size)
	    y = size;
	for (sq = square ; y ; --y, sq += image->width)
	    memset(sq, pixel, n);
	return TRUE;
    }

    n = size >> 1;
    return decodesquare(ypos, xpos, n, cb, pixel)
	&& decodesquare(ypos, xpos + n, n, cb, pixel)
	&& decodesquare(ypos + n, xpos, n, cb, pixel)
	&& decodesquare(ypos + n, xpos + n, n, cb, pixel);
}

/*
 * exported functions
 */

int readprfrow(fileinfo *f, imageinfo *i)
{
    int	x, z;

    if (i->ypos >= i->height)
	return 0;
    file = f;
    image = i;
    for (z = 0 ; z < image->planes ; ++z) {
	imagebuf = image->buf[z];
	for (x = 0 ; x < image->width ; x += squaresize)
	    if (!decodesquare(image->ypos, x, squaresize, image->bits, 0))
		return -1;
    }
    return 1;
}

int readprfheader(fileinfo *f, imageinfo *i)
{
    uchar	buf[13];
    int		n, z;

    if (fread(buf, 13, 1, f->fp) != 1)
	return fileerr(f);

    if (buf[0] == 'M')
	f->type = mrf;
    else if (buf[0] == 'P')
	f->type = prf;
    else
	return err("invalid prf signature");
    if (buf[1] != 'R' || buf[2] != 'F' || buf[3] != '1')
	return err("invalid prf signature");

    i->width = (buf[4] << 24) | (buf[5] << 16) | (buf[6] << 8) | buf[7];
    i->height = (buf[8] << 24) | (buf[9] << 16) | (buf[10] << 8) | buf[11];
    i->bits = (buf[12] & 31) + 1;
    i->planes = ((buf[12] >> 5) & 7) + 1;

    if (i->width <= 0 || i->height <= 0)
	return err("invalid prf image size");
    if (i->bits > 8)
	return err("prf pixel size incompatible with compressed pnm");
    if (i->planes != 1 && i->planes != 3)
	return err("prf plane count incompatible with pnm");

    n = i->width * squaresize;
    if (!(i->buf[0] = calloc(n, i->planes)))
	return err(NULL);
    for (z = 1 ; z < i->planes ; ++z)
	i->buf[z] = i->buf[z - 1] + n;
    i->ypos = 0;

    return TRUE;
}
