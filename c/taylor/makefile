
CFLAGS := -std=c99 -Wall -Wextra 

all: taylor

taylor: taylor.c
	${CC} ${CFLAGS} $< -lm -o $@

clean:
	rm -f taylor
