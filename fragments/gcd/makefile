OBJFIL=main.o gcd.o
TARGET=gcd

all: $(TARGET)

$(TARGET): $(OBJFIL)
	cc $^ -o $@;

clean:
	rm -vf $(TARGET) *.o;
