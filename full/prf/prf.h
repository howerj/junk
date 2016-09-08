/* Written by Brian Raiter, January 2001. Placed in the public domain. */

#ifndef	_prf_h_
#define	_prf_h_

#include	"gen.h"

extern int readprfheader(fileinfo *file, imageinfo *image);
extern int readprfrow(fileinfo *file, imageinfo *image);

extern int writeprfrow(fileinfo *file, imageinfo *image);
extern int writeprfheader(fileinfo *file, imageinfo *image);

#endif
