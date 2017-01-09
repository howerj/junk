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

unit: unit.o libcompress.a
	${CC} ${CFLAGS} $^ -o $@

test: unit
	./$<

lzss: lzss.c io.o
	${CC} ${CFLAGS} -DLZSS_MAIN $^ -o $@

# Example run of lzss program
lzss.out: lzss
	./lzss e lzss.c lzss.c.out
	./lzss d lzss.c.out lzss.out
	cmp lzss.c lzss.out

clean:
	rm -f ${TARGET} *.a *.o *.lzss *.rle *.out *.log
