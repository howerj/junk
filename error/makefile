CC=gcc
CCOPTS=-ansi -g -Wall -Wno-write-strings -Wshadow -Wextra -pedantic -O2

all: main.c error.o
	$(CC) $(CCOPTS) main.c error.o -o error

error.o: error.c error.h
	$(CC) $(CCOPTS) -c error.c -o error.o

clean:
	@rm -vf error *.o *.swp *.swo
