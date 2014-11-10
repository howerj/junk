BINDIR = /usr/local/bin
MANDIR = /usr/local/man

CC = gcc
CFLAGS = -Wall -ansi -W -O2
LDFLAGS = -s

#### rules ####

all: pnmtoprf prftopnm prffile

pnmtoprf   : pnmtoprf.o gen.o readpnm.o writeprf.o
prftopnm   : prftopnm.o gen.o readprf.o writepnm.o
prffile    : prffile.c

gen.o      : gen.c gen.h
readpnm.o  : readpnm.c gen.h pnm.h
writepnm.o : writepnm.c gen.h pnm.h
readprf.o  : readprf.c gen.h prf.h
writeprf.o : writeprf.c gen.h prf.h
pnmtoprf.o : pnmtoprf.c gen.h pnm.h prf.h
prftopnm.o : prftopnm.c gen.h prf.h pnm.h

install: all
	install pnmtoprf prftopnm prffile $(BINDIR)
	install pnmtoprf.1 prftopnm.1 prffile.1 $(MANDIR)/man1
	install prf.5 $(MANDIR)/man5

clean:
	rm -f pnmtoprf prftopnm prffile pnmtoprf.o prftopnm.o gen.o
	rm -f readpnm.o writepnm.o readprf.o writeprf.o
