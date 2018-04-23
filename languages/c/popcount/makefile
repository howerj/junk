OBJFIL=main.o popcount.o
TARGET=popcount

all: $(TARGET)

$(TARGET): $(OBJFIL)
	cc $^ -o $@;

clean:
	rm -vf $(TARGET) *.o;
