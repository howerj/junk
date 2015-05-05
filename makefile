CC=gcc
CFLAGS=-O2 -Wall -Wextra -g 
TARGET=emu
OBJECTS=emu.o main.o srec.o uart.o util.o
.PHONY: all run clean
all: $(TARGET)
%.o: %.c *.h
	$(CC) $(CFLAGS) -c -o $@ $<
$(TARGET): $(OBJECTS) mips.h internal.h
	$(CC) $(CFLAGS) $(OBJECTS) -lpthread -o $@
run: $(TARGET)
	./$(TARGET) vmlinux.srec
clean:
	rm -fv *.o ./$(TARGET)
