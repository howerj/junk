prolog:miniprolog.c y.tab.c lex.yy.c printFunctions.c
	gcc miniprolog.c y.tab.c lex.yy.c printFunctions.c -o prolog -lfl
y.tab.c: prolog.y
	bison -y -d -g -t --debug prolog.y
lex.yy.c: prolog.l
	lex prolog.l

clean:
	rm -f lex.yy.c y.tab.c y.tab.h y.dot y.output prolog
