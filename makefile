TARGET=ebnf
CC=gcc
CFLAGS=-Wall -Wextra -std=c99 
all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) $^ -o $@

run: $(TARGET)
	./$(TARGET) ebnf.bnf

clean:
	-rm -rf $(TARGET) *.a *.so *.o
