TARGET=compress
AR      := ar
CFLAGS  ?= -Wall -Wextra -std=c99
CC      ?= gcc
TARGET  = compress

all: ${TARGET}

libcompress.a: rle.o lzss.o io.o
	${AR} rcs $@ $^

${TARGET}: libcompress.a main.o
	${CC} ${CFLAGS} $^ -o $@

clean:
	rm -f ${TARGET} *.a *.o
