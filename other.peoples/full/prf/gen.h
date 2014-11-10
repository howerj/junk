/* Written by Brian Raiter, January 2001. Placed in the public domain. */

#ifndef	_gen_h_
#define	_gen_h_

#include	<stdio.h>

#ifndef	TRUE
#define	TRUE		1
#define	FALSE		0
#endif

typedef	unsigned char	_uchar;
#undef uchar
#define	uchar		_uchar

enum {
    pbmasc = '1', pgmasc = '2', ppmasc = '3',
    pbmraw = '4', pgmraw = '5', ppmraw = '6',
    mrf = 'M', prf = 'P'
};

typedef	struct fileinfo {
    int		type;
    char const *name;
    FILE       *fp;
    int		bitcount;
    int		byte;
} fileinfo;

typedef	struct imageinfo {
    int		width;
    int		height;
    int		bits;
    int		planes;
    uchar      *buf[4];
    int		ypos;
} imageinfo;

enum { squaresize = 64 };

extern char const *progname;

extern int err(char const *msg);
extern int fileerr(fileinfo const *file);
extern int openreadfile(fileinfo *file, char const *name, FILE *fp);
extern int openwritefile(fileinfo *file, char const *name, FILE *fp);
extern int closereadfile(fileinfo *file);
extern int closewritefile(fileinfo *file);

#endif
