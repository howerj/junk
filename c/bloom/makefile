all: bloom

bloom: bloom.o test.o
	$(CC) -o bloom -Wall -pedantic bloom.o test.o

bloom.o: bloom.c bloom.h
	$(CC) -o bloom.o -Wall -pedantic -ansi -c bloom.c

test.o: test.c bloom.h
	$(CC) -o test.o -Wall -pedantic -ansi -c test.c
