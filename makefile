TARGET=ebnf
CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -g
all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) $^ -o $@

run: $(TARGET)
	./$(TARGET) ebnf.bnf

valgrind: $(TARGET)
	valgrind ./$(TARGET) ebnf.bnf

clean:
	-rm -rf $(TARGET) *.a *.so *.o
