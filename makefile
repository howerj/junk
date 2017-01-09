TARGET=compress
AR      := ar
# -pg -g -fprofile-arcs -ftest-coverage
CFLAGS  ?= -Wall -Wextra -std=c99 -pedantic 
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

zeros.data: /dev/zero
	dd if=$< bs=1024 count=1024 of=$@

zeros.lzss: lzss zeros.data
	time ./$< e zeros.data $@

clean:
	rm -f ${TARGET} *.a *.o *.lzss *.rle *.out *.log *.data
