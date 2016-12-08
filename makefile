TARGET=compress
AR      := ar
CFLAGS  ?= -Wall -Wextra -std=c99
CC      ?= gcc
TARGET  = compress

all: ${TARGET} unit

libcompress.a: rle.o lzss.o io.o
	${AR} rcs $@ $^

${TARGET}: libcompress.a main.o
	${CC} ${CFLAGS} $^ -o $@

unit: libcompress.a unit.o
	${CC} ${CFLAGS} $^ -o $@

test: unit
	./$<

clean:
	rm -f ${TARGET} *.a *.o
