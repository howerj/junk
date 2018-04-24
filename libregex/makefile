CFLAGS=-Wall -Wextra -std=c99 -g 
CC=gcc

all: grep

grep: grep.o regex.o 

%.o:%.c *.h
	@echo cc $< -c -o $@
	@$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -f *.o grep
