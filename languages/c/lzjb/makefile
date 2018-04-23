CC=gcc
CFLAG=-Wall -Wextra

.PHONY: all clean

all: compress

main.o: main.c 
	$(CC) $(CFLAG) -c $< -o $@

compress.o: compress.c compress.h
	$(CC) $(CFLAG) -c $< -o $@

compress: main.o compress.o
	$(CC) $(CFLAG) $^ -o $@

clean:
	rm -f *.o *~ compress
