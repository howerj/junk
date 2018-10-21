# Modified from the original

This is not the original project but a personal copy of it.  
The original is at <https://github.com/andrewchambers/cmips>.

I have changed the API, ported the VM to Windows and restructured
the project (which are all minor things compared to the original
implementation). 

The original README.md (with a few modifications) is below:

# cmips

Tiny MIPS 4kc emulator (smallest and easiest emulator to hack on that I
know of - It boots Linux in a few thousands lines of C code.)  I wrote
this emulator in the month or so after I graduated university and before
I started work.

Let me know if it has helped you in any way, I appreciate any feedback.

# Building and Running

How to compile and run the emulator.

I have not put my custom Linux kernel board support code online yet,
so I included a precompiled kernel in the images folder. I have tested
on Linux (main platform) and Cygwin in windows.


# Motivations

1. To test my skills and learn about processors and operating systems.

2. To compile to javascript (asm.js) with emscripten to test it vs Fabrice Bellard's 
<http://bellard.org/jslinux/> (I started this, but did not finish writing a terminal 
emulator.)

3. To embed in other applications like games, simulators and sandboxes so 
people can have a realistic processor to play with.  It doesn't really 
have any dependencies, so if you rewrite the serial port code, and 
embed the step function in your program loop somehow, it will work very easily.

# Status

Currently boots a custom built Linux kernel. Timers don not work quite right,
so calculating bogomips takes a while during boot, and time readings are off.

It currently supports a UART chip, and uses the CPU timer.

It may eventually support pluggable framebuffer, MTD flash, 8250 UART, 
Real Time Clock  if there are reasons to extend it.

It uses the Linux emulation of floating point instructions.

There is no way to quit the emulator currently with the keyboard because 
I set the terminal to raw mode and was lazy.

(Pull requests and issues welcome.)

# Tests

Test suite I used for bootstrapping located at <https://github.com/andrewchambers/met>.

The Linux kernel itself is more comprehensive.

I used creduce and csmith as a way of fuzzing my emulator initially too.

# Misc

The disgen.py program is just a python script which converts a JSON
representation of the opcodes into a giant switch case for disassembling
and executing.

# Info

I used buildroot to build the image, and wrote a custom Linux kernel board 
support package for the mips kernel following:

<http://linux.junsun.net/porting-howto/>

Unfortunately it was a bit out of date, so I had to improvise.

Those who wish to try similar projects, I learnt a lot by working with
my friend on a predecessor javascript emulator and a gameboy emulator.

I mainly used the mips 4kc manual I got from Wikipedia, and I read parts
of the qemu code if things were not totally clear.

The uart code is inspired from <http://s-macke.github.io/jor1k/>.

The people on #linux-mips also helped me when I hit a bug in the MIPS linux 
kernel which prevented booting.
