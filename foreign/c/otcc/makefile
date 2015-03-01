CC=gcc
CFLAGS=-Wall -Wextra
TARGETS=otcc otccelf otccelfn otccn otccex
all: $(TARGETS)
otcc: otcc.c
	$(CC) -ldl $@.c -o $@ 
otccn: otccn.c
	$(CC) -ldl $@.c -o $@ 
clean:
	rm -f $(TARGETS) *.o
