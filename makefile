TARGET=ebnf

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) $^ -o $@

run: $(TARGET)
	./$(TARGET) ebnf.bnf

clean:
	-rm -rf $(TARGET) *.a *.so *.o
