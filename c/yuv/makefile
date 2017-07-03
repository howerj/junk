CFLAGS=-Wall -Wextra -std=c99

all: yuvl

life.yuv: yuvl
	-./$< | dd bs=1024 count=131072 of=$@

clean:
	rm -fv yuvl
	rm -fv life.yuv

