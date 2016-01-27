TARGET=ebnf

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	-rm -rf $(TARGET) *.a *.so *.o
