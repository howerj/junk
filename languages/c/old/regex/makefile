#
# Regex: Makefile
# @author         Richard James Howe.
# @email          howe.r.j.89@gmail.com
#


## Colors

BLUE=\e[1;34m
GREEN=\e[1;32m
RED=\e[1;31m
DEFAULT=\e[0m

## Compiler options

CC=gcc
CCOPTS=-ansi -g -Wall -Wno-write-strings -Wshadow -Wextra -pedantic -O2
#CCOPTS=-ansi --coverage -g -Wall -Wno-write-strings -Wshadow -Wextra -pedantic -O2

## Help

all: banner regex run

banner:
	@/bin/echo -e "Regex, GNU Makefile\n"
	@/bin/echo -e "Author:    $(BLUE)Richard James Howe$(DEFAULT)."
	@/bin/echo -e "Email:     $(BLUE)howe.r.j.89@gmail.com$(DEFAULT)."
	@/bin/echo -e "\n"


help:
	@/bin/echo "Options:"
	@/bin/echo "make"
	@/bin/echo "     Print out banner, this help message and compile program."
	@/bin/echo "make help"
	@/bin/echo "     This help message."
	@/bin/echo "make run"
	@/bin/echo "		  Run the program."
	@/bin/echo "make regex"
	@/bin/echo "     Compile regex."
	@/bin/echo "make pretty"
	@/bin/echo "     Indent source, print out word count, clean up directory."
	@/bin/echo "make clean"
	@/bin/echo "     Clean up directory."
	@/bin/echo "make gcov"
	@/bin/echo "     Compile program with coverage options, run program, run gcov."
	@/bin/echo "make splint"
	@/bin/echo "     Run splint on all *.c and *.h source files."
	@/bin/echo "make html"
	@/bin/echo "     Compile the documentation."
	@/bin/echo "make valgrind"
	@/bin/echo "     Run program with valgrind on start up regex file only."

run: regex
	@/bin/echo "./regex"
#	@./regex

## Main regex program

regex: main.c regex.h regex.o
	$(CC) $(CCOPTS) main.c regex.o -o regex 

regex.o: regex.c regex.h
	$(CC) $(CCOPTS) -c regex.c -o regex.o

## Optional extras, helper functions

# Indent the files, clean up directory, word count.
pretty: 
	@/bin/echo -e "$(BLUE)"
	@/bin/echo -e "Indent files and clean up system.$(DEFAULT)"
	@/bin/echo -e "$(GREEN)"
	@/bin/echo "indent -nut -linux *.h *.c";
	@indent -nut -linux *.h *c;
	@/bin/echo -e "$(RED)"
	@rm -vf regex memory.txt *.log *.swo *.swp *.o *~ *.gcov *.gcda *.gcno *.html *.htm;
	@/bin/echo -e "$(DEFAULT)"
	@wc *.c *.h *.fs makefile

# Clean up directory.
clean:
	@/bin/echo -e "$(RED)"
	@rm -vf regex memory.txt *.log *.swo *.swp *.o *~ *.gcov *.gcda *.gcno *.html *.htm;
	@/bin/echo -e "$(DEFAULT)"

# Static checking.
splint:
	@/bin/echo "Running \"splint *.c *.h &> splint.log\""
	-splint *.c *.h &> splint.log 

html:
	@/bin/echo -e "Compiling markdown to html."
	@for i in *.md; do /bin/echo "$$i > $$i.html"; markdown $$i > $$i.html; done

valgrind: regex
	@/bin/echo "Running valgrind on ./regex"
	@/bin/echo "  This command needs changing in the makefile"
	-valgrind ./regex &> valgrind.log

gcov: CCOPTS:=$(CCOPTS) --coverage
gcov: clean regex
	@/bin/echo "Providing gcov statistics for regex program."
	@./regex
	@gcov regex.c main.c


