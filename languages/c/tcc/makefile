# This exe does not work under valgrind
# GCC might work with the right flags, it does not by default
CC=tcc
# libtcc neads dlopen/dlclose from libdl
CFLAGS=
LINKF=-ldl -ltcc

all: ex1_libtcc

ex1_libtcc: ex1_libtcc.c
	$(CC) $(CFLAGS) $< $(LINKF) -o $@

clean:
	rm -f ex1_libtcc *.o;
