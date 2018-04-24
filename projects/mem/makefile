
.PHONY: all clean

TARGET := mem
SRCS   := ${wildcard *.c}
OBJS   := ${SRCS:%.c=%.o}
INC    := -Ilibline -Impc
CFLAGS := -Wall -Wextra -g -std=c99

all: ${TARGET}

%.o: %.c
	${CC} ${CFLAGS} ${INC} -o $@ -c $<

${TARGET}: ${OBJS} libline.a mpc/mpc.o
	${CC} $^ -o $@

mpc: 
	git submodule update --init --recursive
libline: 
	git submodule update --init --recursive

libline.a: libline
	make -C libline
	cp libline/libline.a .

run: ${TARGET}
	sudo ./${TARGET}

clean:
	rm -f ${TARGET} *.o *.log

