CC=gcc
CFLAGS=-Wall -Wextra
TARGET=tea_test

all: $(TARGET)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

tea_test: tea.o xtea.o xxtea.o main.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o $(TARGET)

