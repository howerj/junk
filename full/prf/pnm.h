/* Written by Brian Raiter, January 2001. Placed in the public domain. */

#ifndef _pnm_h_
#define _pnm_h_

#include	"gen.h"

extern int readpnmheader(fileinfo *file, imageinfo *image);
extern int readpnmrow(fileinfo *file, imageinfo *image);

extern int writepnmheader(fileinfo *file, imageinfo *image);
extern int writepnmrow(fileinfo *file, imageinfo *image);

#endif
