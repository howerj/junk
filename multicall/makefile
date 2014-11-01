OBJFIL=main.o popcount.o gcd.o note.o
TARGET=multicall
CFLAGS=-Wall -Wextra -Os

all: $(TARGET)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJFIL) *.h
	cc $^ -o $@;

clean:
	rm -vf $(TARGET) *.o;
