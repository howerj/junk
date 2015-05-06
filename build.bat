REM This was test on Windows 7 and built with the Tiny C Compiler
REM which is available from <http://bellard.org/tcc/>. Version 0.9.26
REM was used. I have not messed around with ANSI.SYS yet, so I have
REM no instructions on how to make the ANSI escape codes work with
REM the terminal yet.
tcc -Wall -c emu.c
tcc -Wall -c main.c
tcc -Wall -c srec.c
tcc -Wall -c uart.c
tcc -Wall -D__WIN32 -c util.c
tcc -Wall emu.o main.o srec.o uart.o util.o -o mips.exe
REM To run the emulator:
REM mips.exe vmlinux.srec
