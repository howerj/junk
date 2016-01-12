TARGET=bnf

all: bnf

bnf: bnf.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	-rm -rf $(TARGET) *.a *.so *.o
