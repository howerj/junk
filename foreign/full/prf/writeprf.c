/* Written by Brian Raiter, January 2001. Placed in the public domain. */

#include	<stdio.h>
#include	<stdlib.h>
#include	"gen.h"
#include	"prf.h"

static fileinfo	       *file;
static imageinfo       *image;
static uchar const     *imagebuf;

/*
 * encoding functions
 */

static int bitsout(int nbits, uchar val)
{
    int	bit;

    for (bit = 1 << (nbits - 1) ; bit ; bit >>= 1) {
	file->byte <<= 1;
	if (val & bit)
	    file->byte |= 1;
	if (!(++file->bitcount & 7)) {
	    if (fputc(file->byte, file->fp) == EOF)
		return fileerr(file);
	    file->byte = 0;
	}
    }
    return TRUE;
}

static int encodesquare(int ypos, int xpos, int size, int cb)
{
    static int const	bitsize[] = { 0, 1, 2, 2, 3, 3, 3, 3, 4 };
    uchar const	       *square;
    uchar const	       *sq;
    uchar		ones, zeros, bit;
    int			count;
    int			y, x, n;

    if (file->type == prf)
	if (ypos >= image->height || xpos >= image->width)
	    return TRUE;

    square = imagebuf + (ypos % squaresize) * image->width + xpos;

    if (size == 1)
	return bitsout(cb, *square);

    ones = 255;
    zeros = 0;
    sq = square;
    y = image->height - ypos;
    if (y > size)
	y = size;
    for (sq = square ; y ; --y, sq += image->width) {
	for (x = 0 ; x < size && xpos + x < image->width ; ++x) {
	    ones &= sq[x];
	    zeros |= sq[x];
	}
	if (!(ones & 128) && (zeros & 128))  /* optimization: no common bits */
	    break;
    }

    count = 0;
    bit = 128 >> (8 - cb);
    while (bit && ((ones & bit) || !(zeros & bit))) {
	++count;
	bit >>= 1;
    }
    if (!bitsout(bitsize[cb], count))
	return FALSE;
    if (count) {
	cb -= count;
	if (!bitsout(count, *square >> cb))
	    return FALSE;
    }

    if (!cb)
	return TRUE;

    n = size >> 1;
    return encodesquare(ypos, xpos, n, cb)
	&& encodesquare(ypos, xpos + n, n, cb)
	&& encodesquare(ypos + n, xpos, n, cb)
	&& encodesquare(ypos + n, xpos + n, n, cb);
}

/*
 * exported functions
 */

int writeprfrow(fileinfo *f, imageinfo *i)
{
    int	x, z;

    file = f;
    image = i;
    for (z = 0 ; z < image->planes ; ++z) {
	imagebuf = image->buf[z];
	for (x = 0 ; x < image->width ; x += squaresize)
	    if (!encodesquare(image->ypos, x, squaresize, image->bits))
		return FALSE;
    }
    return TRUE;
}

int writeprfheader(fileinfo *f, imageinfo *i)
{
    uchar	buf[13];

    buf[0] = f->type;
    buf[1] = 'R';
    buf[2] = 'F';
    buf[3] = '1';
    buf[4] = i->width >> 24;
    buf[5] = i->width >> 16;
    buf[6] = i->width >> 8;
    buf[7] = i->width;
    buf[8] = i->height >> 24;
    buf[9] = i->height >> 16;
    buf[10] = i->height >> 8;
    buf[11] = i->height;
    buf[12] = ((i->bits - 1) & 31) | (((i->planes - 1) & 7) << 5);

    if (fwrite(buf, 13, 1, f->fp) != 1)
	return fileerr(f);
    return TRUE;
}
