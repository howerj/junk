/* this small string manipulation program
 * showcases different features of the qcc
 * compiler.
 */

/* C style comments are tolerated by the
 * lexer everywhere, no C++ comments are
 * allowed.
 */

/* qcc integers are 64-bits wide so they
 * can represent pointers, they are always
 * signed. Global variables are supported,
 * they are 0 initialized.
 */
int f, len, line;

/* only old style C functions are supported
 */
getln()
{
	/* local variables work just fine
	 */
	int c, p;

	p=line;
	/* you can use regular C style
	 * expressions and while loops
	 */
	while ((c=fgetc(f))<256) {
		/* pointer dereference always
		 * goes with a cast
		 */
		*(char *)p = c;
		len++;
		if (c=='\n')
			/* break statements are
			 * supported
			 */
			break;
		p++;
	}

	*(char *)p = 0;
	if (p==line && c!='\n')
		return 0; /* signal eof */
	return 1; /* there is more to read */
}

r(ch)
{
	if (ch >= 'a' && ch <= 'z')
		ch = 'a' + (ch-'a'+13) % 26;
	if (ch >= 'A' && ch <= 'Z')
		ch = 'A' + (ch-'A'+13) % 26;
	return ch;
}

rot()
{
	int p;

	/* for loops work just fine
	 */
	for (p=line; *(char *)p; p++)
		*(char *)p = r(*(char *)p);
}

rev()
{
	int b, e;

	b=line;
	if (*(char *)b == 0)
		return;
	/* external function calls are supported
	 * via dynamic linking
	 */
	e=strchr(b, 0)-1;

	/* since b and e are signed integers, we
	 * cannot test here for b<e
	 */
	while (labs(b-e) > 1) {
		/* each block can have its own private
		 * set of local variables, scoping is done
		 * properly
		 */
		int t;

		t = *(char *)b;
		*(char *)b++ = *(char *)e;
		*(char *)e-- = t;
	}
}

main(c, v)
int c, v;
{
	int n, fp;

	/* external constants are also supported
	 * via dynamic linking
	 */
	f = stdin;
	/* assignment is right associative, as usual
	 */
	fp = n = 0;

	c--; v=8+v;
	while (c--) {
		int o, opt;

		opt = *(int *)v;
		if (*(char *)opt == '-')
			while (1) {
				/* function pointers work as in C
				 */
				opt++;
				o = *(char *)opt;

				     if (o == 'e')
					fp = &rot;
				else if (o == 'r')
					fp = &rev;
				else if (o == 'n')
					n = 1;
				else if (o == 0)
					break;
				else
					/* forward references are
					 * allowed, and so is
					 * recursion
					 */
					usage();
			}
		else
			f = fopen(opt, "r");

		v=8+v; /* next option */
	}

	/* malloc is just an external function call
	 * resolved by the dynamic linker
	 */
	line = malloc(4096);
	while (getln()) {
		if (fp)
			/* call our function pointer to transform
			 * the current line!
			 */
			(*(int (*)())fp)();
		puts(line);
	}
	free(line);

	if (n)
		fprintf(stderr, "%d bytes processed\n", len);

	return 0;
}

usage()
{
	/* string literals are supported
	 */
	puts("usage: qccx [OPTION]... [FILE]");
	puts("  Transform a text file and print the result,");
	puts("  if no FILE is given the standard input is used.");
	puts("  Options can be accumulated (e.g. -rn).");
	puts("");
	puts("  -e rot13 encode/decode");
	puts("  -r reverse lines");
	puts("  -n print statistics");
	exit(0);
}
