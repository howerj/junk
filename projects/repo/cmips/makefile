CC=gcc
CFLAGS=-O2 -Wall -Wextra
TARGET=mips
.PHONY: all run clean
all: $(TARGET)
%.o: %.c *.h
	$(CC) $(CFLAGS) -c -o $@ $<
lib$(TARGET).a: emu.o srec.o uart.o util.o 
	ar rcs $@ $^
lib$(TARGET).so: emu.c srec.c uart.c util.c util.h internal.h mips.h
	$(CC) $(CFLAGS) -fpic -shared $^ -o $@
$(TARGET): main.o lib$(TARGET).a
	$(CC) $(CFLAGS) main.o lib$(TARGET).a -lpthread -o $@
run: $(TARGET)
	./$(TARGET) vmlinux.srec
clean:
	rm -fv *.o *.a *.so ./$(TARGET)
