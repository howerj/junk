CFLAGS=-std=c99 -Wall -Wextra -pedantic -g
TRACER=

all: concat strntol bits timer

run: concat strntol bits pack.o
	${TRACER} ./concat
	${TRACER} ./strntol
	${TRACER} ./bits

clean:
	rm -fv concat strntol bits timer


