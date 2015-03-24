/* Program 'picoProlog' */

#include "ptclib.i"

boolean run;
boolean dflag;

typedef integer permstring;

typedef char tempstring[128];

integer charptr;
char charbuf[2048];

integer StringLength(s)
tempstring s;
{
     integer i;

     integer __R__;

     i = 0;
     while ((s[(i + 1) - 1] != chr(0)))
          i = (i + 1);
     __R__ = i;
     return __R__;
}

permstring SaveString(s)
tempstring s;
{
     integer i;

     permstring __R__;

     if ((((charptr + StringLength(s)) + 1) > 2048)) {
          putc('\n', output);
          fprintf(output, "Panic: out of string space\n");
          halt();
     }
     __R__ = (charptr + 1);
     i = 0;
     do {
          i = (i + 1);
          charptr = (charptr + 1);
          charbuf[charptr - 1] = s[i - 1];
     } while (! ((s[i - 1] == chr(0))));
     return __R__;
}

boolean StringEqual(s1, s2)
tempstring s1;
permstring s2;
{
     integer i;

     boolean __R__;

     i = 1;
     while (((s1[i - 1] != chr(0)) && (s1[i - 1] == charbuf[((s2 + i)
               - 1) - 1])))
          i = (i + 1);
     __R__ = (s1[i - 1] == charbuf[((s2 + i) - 1) - 1]);
     return __R__;
}

void WriteString(s)
permstring s;
{
     integer i;

     i = s;
     while ((charbuf[i - 1] != chr(0))) {
          putc(charbuf[i - 1], output);
          i = (i + 1);
     }
}

integer lsp, gsp, hp, hmark;
integer mem[1000000];

integer LocAlloc(size)
integer size;
{
     integer __R__;

     if (((lsp + size) >= gsp)) {
          putc('\n', output);
          fprintf(output, "Panic: out of stack space\n");
          halt();
     }
     __R__ = (lsp + 1);
     lsp = (lsp + size);
     return __R__;
}

integer GloAlloc(kind, size)
integer kind;
integer size;
{
     integer p;

     integer __R__;

     if (((gsp - size) <= lsp)) {
          putc('\n', output);
          fprintf(output, "Panic: out of stack space\n");
          halt();
     }
     gsp = (gsp - size);
     p = gsp;
     mem[p - 1] = ((256 * kind) + size);
     __R__ = p;
     return __R__;
}

integer HeapAlloc(size)
integer size;
{
     integer __R__;

     if (((hp + size) > 1000000)) {
          putc('\n', output);
          fprintf(output, "Panic: out of heap space\n");
          halt();
     }
     __R__ = (hp + 1);
     hp = (hp + size);
     return __R__;
}

boolean interacting;
char pbchar;
text infile;
integer lineno;
permstring filename;

char FGetChar(f)
text f;
{
     char ch;

     char __R__;

     if (eof(f))
          __R__ = chr(127);
     else if (eoln(f)) {
          skipln(f);
          lineno = (lineno + 1);
          __R__ = chr(10);
     }
     else {
          ch = getc(f);
          __R__ = ch;
     }
     return __R__;
}

char GetChar()
{
     char __R__;

     if ((pbchar != chr(127))) {
          __R__ = pbchar;
          pbchar = chr(127);
     }
     else if (interacting)
          __R__ = FGetChar(input);
     else
          __R__ = FGetChar(infile);
     return __R__;
}

void PushBack(ch)
char ch;
{
     pbchar = ch;
}

integer current;
integer call;
integer goalframe;
integer choice;
integer base;
integer proc;

integer Deref(t, e)
integer t;
integer e;
{
     integer __R__;

     if ((t == 0)) {
          putc('\n', output);
          fprintf(output, "Panic: Deref\n");
          halt();
     }
     if ((((mem[t - 1] / 256) == 5) && (e != 0)))
          t = ((e + 7) + ((mem[(t + 2) - 1] - 1) * 3));
     while ((((mem[t - 1] / 256) == 4) && (mem[(t + 2) - 1] != 0)))
          t = mem[(t + 2) - 1];
     __R__ = t;
     return __R__;
}

void PrintTerm();

integer ParseTerm();

boolean DoBuiltin();

void Collect();

integer Key();

typedef integer symbol;

integer nsymbols;
struct {
     integer name;
     integer arity;
     integer action;
     integer proc;
} symtab[511];
symbol cons, eqsym, cutsym, nilsym, notsym;

symbol Lookup(name)
tempstring name;
{
     integer h, i;
     symbol p;

     symbol __R__;

     h = 0;
     i = 1;
     while ((name[i - 1] != chr(0))) {
          h = (((5 * h) + ord(name[i - 1])) % 511);
          i = (i + 1);
     }
     p = (h + 1);
     while ((symtab[p - 1].name != (- 1))) {
          if (StringEqual(name, symtab[p - 1].name))
               goto L1;
          p = (p - 1);
          if ((p == 0))
               p = 511;
     }
     if ((nsymbols >= ((511 / 10) * (90 / 10)))) {
          putc('\n', output);
          fprintf(output, "Panic: out of symbol space\n");
          halt();
     }
     nsymbols = (nsymbols + 1);
     symtab[p - 1].name = SaveString(name);
     symtab[p - 1].arity = (- 1);
     symtab[p - 1].action = 0;
     symtab[p - 1].proc = 0;
L1:
     __R__ = p;
     return __R__;
}

typedef char keyword[8];

symbol Enter(name, arity, action)
keyword name;
integer arity;
integer action;
{
     symbol s;
     integer i;
     tempstring temp;

     symbol __R__;

     i = 1;
     while ((name[i - 1] != ' ')) {
          temp[i - 1] = name[i - 1];
          i = (i + 1);
     }
     temp[i - 1] = chr(0);
     s = Lookup(temp);
     symtab[s - 1].arity = arity;
     symtab[s - 1].action = action;
     __R__ = s;
     return __R__;
}

void InitSymbols()
{
     integer i;
     symbol dummy;

     nsymbols = 0;
     for (i = 1; i <= 511; i++)
          symtab[i - 1].name = (- 1);
     cons = Enter(":       ", 2, 0);
     cutsym = Enter("!       ", 0, 1);
     eqsym = Enter("=       ", 2, 8);
     nilsym = Enter("nil     ", 0, 0);
     notsym = Enter("not     ", 1, 7);
     dummy = Enter("call    ", 1, 2);
     dummy = Enter("plus    ", 3, 3);
     dummy = Enter("times   ", 3, 4);
     dummy = Enter("integer ", 1, 5);
     dummy = Enter("char    ", 1, 6);
     dummy = Enter("false   ", 0, 9);
     dummy = Enter("print   ", 1, 10);
     dummy = Enter("nl      ", 0, 11);
}

void AddClause(c)
integer c;
{
     symbol s;
     integer p;

     s = mem[(mem[(c + 3) - 1] + 2) - 1];
     if ((symtab[s - 1].action != 0)) {
          {
               putc('\n', output);
               fprintf(output,
                         "Error: can\'t add clauses to built-in relation ");
               run = FALSE;
          }
          WriteString(symtab[s - 1].name);
     }
     else if ((symtab[s - 1].proc == 0))
          symtab[s - 1].proc = c;
     else {
          p = symtab[s - 1].proc;
          while ((mem[(p + 2) - 1] != 0))
               p = mem[(p + 2) - 1];
          mem[(p + 2) - 1] = c;
     }
}

typedef integer argbuf[63];

integer MakeCompound(fun, arg)
symbol fun;
argbuf arg;
{
     integer p;
     integer i, n;

     integer __R__;

     n = symtab[fun - 1].arity;
     p = HeapAlloc((3 + n));
     mem[p - 1] = (((256 * 1) + 3) + n);
     mem[(p + 2) - 1] = fun;
     for (i = 1; i <= n; i++)
          mem[((p + i) + 2) - 1] = arg[i - 1];
     __R__ = p;
     return __R__;
}

integer MakeNode(fun, a1, a2)
symbol fun;
integer a1;
integer a2;
{
     argbuf arg;

     integer __R__;

     arg[1 - 1] = a1;
     arg[2 - 1] = a2;
     __R__ = MakeCompound(fun, arg);
     return __R__;
}

integer refnode[63];

integer MakeRef(offset)
integer offset;
{
     integer __R__;

     __R__ = refnode[offset - 1];
     return __R__;
}

integer MakeInt(i)
integer i;
{
     integer p;

     integer __R__;

     p = HeapAlloc(3);
     mem[p - 1] = ((256 * 2) + 3);
     mem[(p + 2) - 1] = i;
     __R__ = p;
     return __R__;
}

integer MakeChar(c)
char c;
{
     integer p;

     integer __R__;

     p = HeapAlloc(3);
     mem[p - 1] = ((256 * 3) + 3);
     mem[(p + 2) - 1] = ord(c);
     __R__ = p;
     return __R__;
}

integer MakeString(s)
tempstring s;
{
     integer p;
     integer i;

     integer __R__;

     i = StringLength(s);
     p = MakeNode(nilsym, 0, 0);
     while ((i > 0)) {
          p = MakeNode(cons, MakeChar(s[i - 1]), p);
          i = (i - 1);
     }
     __R__ = p;
     return __R__;
}

integer MakeClause(nvars, head, body, nbody)
integer nvars;
integer head;
argbuf body;
integer nbody;
{
     integer p;
     integer i;

     integer __R__;

     p = HeapAlloc(((4 + nbody) + 1));
     mem[p - 1] = nvars;
     mem[(p + 2) - 1] = 0;
     mem[(p + 3) - 1] = head;
     for (i = 1; i <= nbody; i++)
          mem[(((p + 4) + i) - 1) - 1] = body[i - 1];
     mem[((((p + 4) + nbody) + 1) - 1) - 1] = 0;
     if ((head == 0))
          mem[(p + 1) - 1] = 0;
     else
          mem[(p + 1) - 1] = Key(head, 0);
     __R__ = p;
     return __R__;
}

boolean IsString(t, e)
integer t;
integer e;
{
     integer i;

     boolean __R__;

     i = 0;
     t = Deref(t, e);
     while ((i < 128)) {
          if ((((mem[t - 1] / 256) != 1) || (mem[(t + 2) - 1] !=
                    cons)))
               goto L3;
          else if (((mem[Deref(mem[((t + 1) + 2) - 1], e) - 1] / 256)
                    != 3))
               goto L3;
          else {
               i = (i + 1);
               t = Deref(mem[((t + 2) + 2) - 1], e);
          }
     }
L3:
     __R__ = (((mem[t - 1] / 256) == 1) && (mem[(t + 2) - 1] ==
               nilsym));
     return __R__;
}

void ShowString(t, e)
integer t;
integer e;
{
     t = Deref(t, e);
     putc('\"', output);
     while ((mem[(t + 2) - 1] != nilsym)) {
          putc(chr(mem[(Deref(mem[((t + 1) + 2) - 1], e) + 2) - 1]),
                    output);
          t = Deref(mem[((t + 2) + 2) - 1], e);
     }
     putc('\"', output);
}

void PrintCompound(t, e, prio)
integer t;
integer e;
integer prio;
{
     symbol f;
     integer i;

     f = mem[(t + 2) - 1];
     if ((f == cons)) {
          if (IsString(t, e))
               ShowString(t, e);
          else {
               if ((prio < 1))
                    putc('(', output);
               PrintTerm(mem[((t + 1) + 2) - 1], e, (1 - 1));
               putc(':', output);
               PrintTerm(mem[((t + 2) + 2) - 1], e, 1);
               if ((prio < 1))
                    putc(')', output);
          }
     }
     else if ((f == eqsym)) {
          if ((prio < 2))
               putc('(', output);
          PrintTerm(mem[((t + 1) + 2) - 1], e, (2 - 1));
          fprintf(output, " = ");
          PrintTerm(mem[((t + 2) + 2) - 1], e, (2 - 1));
          if ((prio < 2))
               putc(')', output);
     }
     else if ((f == notsym)) {
          fprintf(output, "not ");
          PrintTerm(mem[((t + 1) + 2) - 1], e, 2);
     }
     else {
          WriteString(symtab[f - 1].name);
          if ((symtab[f - 1].arity > 0)) {
               putc('(', output);
               PrintTerm(mem[((t + 1) + 2) - 1], e, 2);
               for (i = 2; i <= symtab[f - 1].arity; i++) {
                    fprintf(output, ", ");
                    PrintTerm(mem[((t + i) + 2) - 1], e, 2);
               }
               putc(')', output);
          }
     }
}

void PrintTerm(t, e, prio)
integer t;
integer e;
integer prio;
{
     t = Deref(t, e);
     if ((t == 0))
          fprintf(output, "*null-term*");
     else {
          switch ((mem[t - 1] / 256)) {
          case 1:
               PrintCompound(t, e, prio);
               break;
          case 2:
               fprintf(output, "%d", mem[(t + 2) - 1]);
               break;
          case 3:
               fprintf(output, "%c%c%c", '\'', chr(mem[(t + 2) - 1]),
                         '\'');
               break;
          case 4:
               if ((t >= gsp))
                    fprintf(output, "%c%d", 'G', ((1000000 - t) / 3));
               else
                    fprintf(output, "%c%d", 'L', ((t - hp) / 3));
               break;
          case 5:
               fprintf(output, "%c%d", '@', mem[(t + 2) - 1]);
               break;
          default:
               fprintf(output, "*unknown-term(tag=%d)*", (mem[t - 1]
                         / 256));
          }
     }
}

void PrintClause(c)
integer c;
{
     integer i;

     if ((c == 0))
          fprintf(output, "*null-clause*\n");
     else {
          if ((mem[(c + 3) - 1] != 0)) {
               PrintTerm(mem[(c + 3) - 1], 0, 2);
               putc(' ', output);
          }
          fprintf(output, ":- ");
          if ((mem[(((c + 4) + 1) - 1) - 1] != 0)) {
               PrintTerm(mem[(((c + 4) + 1) - 1) - 1], 0, 2);
               i = 2;
               while ((mem[(((c + 4) + i) - 1) - 1] != 0)) {
                    fprintf(output, ", ");
                    PrintTerm(mem[(((c + 4) + i) - 1) - 1], 0, 2);
                    i = (i + 1);
               }
          }
          fprintf(output, "%c\n", '.');
     }
}

integer token;
symbol tokval;
integer tokival;
tempstring toksval;
boolean errflag;
integer errcount;

void ShowError()
{
     errflag = TRUE;
     errcount = (errcount + 1);
     if ((! interacting)) {
          putc('\"', output);
          WriteString(filename);
          fprintf(output, "\", line %d%c", lineno, ' ');
     }
     fprintf(output, "Syntax error - ");
}

void Recover()
{
     char ch;

     if (((! interacting) && (errcount >= 20))) {
          fprintf(output, "Too many errors: I\'m giving up\n");
          halt();
     }
     if ((token != 10)) {
          do {
               ch = GetChar();
          } while (! ((((ch == '.') || (ch == chr(127))) ||
                    (interacting && (ch == chr(10))))));
          token = 10;
     }
}

void Scan()
{
     char ch, ch2;
     integer i;

     ch = GetChar();
     token = 0;
     while ((token == 0)) {
          if ((ch == chr(127)))
               token = 14;
          else if ((((ch == ' ') || (ch == chr(9))) || (ch ==
                    chr(10))))
               ch = GetChar();
          else if (((((ch >= 'A') && (ch <= 'Z')) || (ch == '_')) ||
                    ((ch >= 'a') && (ch <= 'z')))) {
               if ((((ch >= 'A') && (ch <= 'Z')) || (ch == '_')))
                    token = 2;
               else
                    token = 1;
               i = 1;
               while ((((((ch >= 'A') && (ch <= 'Z')) || (ch == '_'))
                         || ((ch >= 'a') && (ch <= 'z'))) || ((ch >=
                         '0') && (ch <= '9')))) {
                    if ((i > 128)) {
                         putc('\n', output);
                         fprintf(output,
                                   "Panic: identifier too long\n");
                         halt();
                    }
                    toksval[i - 1] = ch;
                    ch = GetChar();
                    i = (i + 1);
               }
               PushBack(ch);
               toksval[i - 1] = chr(0);
               tokval = Lookup(toksval);
               if ((tokval == notsym))
                    token = 13;
          }
          else if (((ch >= '0') && (ch <= '9'))) {
               token = 3;
               tokival = 0;
               while (((ch >= '0') && (ch <= '9'))) {
                    tokival = ((10 * tokival) + (ord(ch) - ord('0')));
                    ch = GetChar();
               }
               PushBack(ch);
          }
          else {
               switch (ch) {
               case '(':
                    token = 7;
                    break;
               case ')':
                    token = 8;
                    break;
               case ',':
                    token = 9;
                    break;
               case '.':
                    token = 10;
                    break;
               case '=':
                    token = 12;
                    break;
               case '!':
                    {
                         token = 1;
                         tokval = cutsym;
                    }
                    break;
               case '/':
                    {
                         ch = GetChar();
                         if ((ch != '*')) {
                              if ((! errflag)) {
                                   ShowError();
                                   fprintf(output,
                                             "bad token \"/\"\n");
                                   Recover();
                              }
                         }
                         else {
                              ch2 = ' ';
                              ch = GetChar();
                              while (((ch != chr(127)) && (! ((ch2 ==
                                        '*') && (ch == '/'))))) {
                                   ch2 = ch;
                                   ch = GetChar();
                              }
                              if ((ch == chr(127))) {
                                   if ((! errflag)) {
                                        ShowError();
                                        fprintf(output,
                                                  "end of file in comment\n");
                                        Recover();
                                   }
                              }
                              else
                                   ch = GetChar();
                         }
                    }
                    break;
               case ':':
                    {
                         ch = GetChar();
                         if ((ch == '-'))
                              token = 6;
                         else {
                              PushBack(ch);
                              token = 11;
                         }
                    }
                    break;
               case '\'':
                    {
                         token = 4;
                         tokival = ord(GetChar());
                         ch = GetChar();
                         if ((ch != '\'')) {
                              if ((! errflag)) {
                                   ShowError();
                                   fprintf(output, "missing quote\n");
                                   Recover();
                              }
                         }
                    }
                    break;
               case '\"':
                    {
                         token = 5;
                         i = 1;
                         ch = GetChar();
                         while (((ch != '\"') && (ch != chr(10)))) {
                              toksval[i - 1] = ch;
                              ch = GetChar();
                              i = (i + 1);
                         }
                         toksval[i - 1] = chr(0);
                         if ((ch == chr(10))) {
                              {
                                   if ((! errflag)) {
                                        ShowError();
                                        fprintf(output,
                                                  "unterminated string\n");
                                        Recover();
                                   }
                              }
                              PushBack(ch);
                         }
                    }
                    break;
               default:
                    {
                         if ((! errflag)) {
                              ShowError();
                              fprintf(output,
                                        "illegal character \"%c%c\n",
                                        ch, '\"');
                              Recover();
                         }
                    }
               }
          }
     }
}

void PrintToken(t)
integer t;
{
     switch (t) {
     case 1:
          {
               fprintf(output, "identifier ");
               WriteString(symtab[tokval - 1].name);
               /* skip */;
          }
          break;
     case 2:
          {
               fprintf(output, "variable ");
               WriteString(symtab[tokval - 1].name);
               /* skip */;
          }
          break;
     case 3:
          fprintf(output, "number");
          break;
     case 4:
          fprintf(output, "char constant");
          break;
     case 6:
          fprintf(output, "\":-\"");
          break;
     case 7:
          fprintf(output, "\"(\"");
          break;
     case 8:
          fprintf(output, "\")\"");
          break;
     case 9:
          fprintf(output, "\",\"");
          break;
     case 10:
          fprintf(output, "\".\"");
          break;
     case 11:
          fprintf(output, "\":\"");
          break;
     case 12:
          fprintf(output, "\"=\"");
          break;
     case 5:
          fprintf(output, "string constant");
          break;
     default:
          fprintf(output, "unknown token");
     }
}

integer nvars;
symbol vartable[63];

integer VarRep(name)
symbol name;
{
     integer i;

     integer __R__;

     if ((nvars == 63)) {
          putc('\n', output);
          fprintf(output, "Panic: too many variables\n");
          halt();
     }
     i = 1;
     vartable[(nvars + 1) - 1] = name;
     while ((name != vartable[i - 1]))
          i = (i + 1);
     if ((i == (nvars + 1)))
          nvars = (nvars + 1);
     __R__ = MakeRef(i);
     return __R__;
}

boolean ShowAnswer(bindings)
integer bindings;
{
     integer i;
     char ch;

     boolean __R__;

     if ((nvars == 0))
          __R__ = TRUE;
     else {
          for (i = 1; i <= nvars; i++) {
               putc('\n', output);
               WriteString(symtab[vartable[i - 1] - 1].name);
               fprintf(output, " = ");
               PrintTerm(((bindings + 7) + ((i - 1) * 3)), 0, (2 -
                         1));
          }
          if ((! interacting)) {
               putc('\n', output);
               __R__ = FALSE;
          }
          else {
               fprintf(output, " ? ");
               flush();
               if (eoln(input)) {
                    skipln(input);
                    __R__ = FALSE;
               }
               else {
                    ch = getc(input), skipln(input);
                    __R__ = (ch == '.');
               }
          }
     }
     return __R__;
}

void Eat(expected)
integer expected;
{
     if ((token == expected)) {
          if ((token != 10))
               Scan();
     }
     else if ((! errflag)) {
          ShowError();
          fprintf(output, "expected ");
          PrintToken(expected);
          fprintf(output, ", found ");
          PrintToken(token);
          putc('\n', output);
          Recover();
     }
}

integer ParseCompound()
{
     symbol fun;
     argbuf arg;
     integer n;

     integer __R__;

     fun = tokval;
     n = 0;
     Eat(1);
     if ((token == 7)) {
          Eat(7);
          n = 1;
          arg[1 - 1] = ParseTerm();
          while ((token == 9)) {
               Eat(9);
               n = (n + 1);
               arg[n - 1] = ParseTerm();
          }
          Eat(8);
     }
     if ((symtab[fun - 1].arity == (- 1)))
          symtab[fun - 1].arity = n;
     else if ((symtab[fun - 1].arity != n)) {
          if ((! errflag)) {
               ShowError();
               fprintf(output, "wrong number of args\n");
               Recover();
          }
     }
     __R__ = MakeCompound(fun, arg);
     return __R__;
}

integer ParsePrimary()
{
     integer t;

     integer __R__;

     if ((token == 1))
          t = ParseCompound();
     else if ((token == 2)) {
          t = VarRep(tokval);
          Eat(2);
     }
     else if ((token == 3)) {
          t = MakeInt(tokival);
          Eat(3);
     }
     else if ((token == 4)) {
          t = MakeChar(chr(tokival));
          Eat(4);
     }
     else if ((token == 5)) {
          t = MakeString(toksval);
          Eat(5);
     }
     else if ((token == 7)) {
          Eat(7);
          t = ParseTerm();
          Eat(8);
     }
     else {
          {
               if ((! errflag)) {
                    ShowError();
                    fprintf(output, "expected a term\n");
                    Recover();
               }
          }
          t = 0;
     }
     __R__ = t;
     return __R__;
}

integer ParseFactor()
{
     integer t;

     integer __R__;

     t = ParsePrimary();
     if ((token != 11))
          __R__ = t;
     else {
          Eat(11);
          __R__ = MakeNode(cons, t, ParseFactor());
     }
     return __R__;
}

integer ParseTerm()
{
     integer t;

     integer __R__;

     t = ParseFactor();
     if ((token != 12))
          __R__ = t;
     else {
          Eat(12);
          __R__ = MakeNode(eqsym, t, ParseFactor());
     }
     return __R__;
}

void CheckAtom(a)
integer a;
{
     if (((mem[a - 1] / 256) != 1)) {
          if ((! errflag)) {
               ShowError();
               fprintf(output, "literal must be a compound term\n");
               Recover();
          }
     }
}

integer ParseClause(isgoal)
boolean isgoal;
{
     integer head, t;
     argbuf body;
     integer n;
     boolean minus;

     integer __R__;

     if (isgoal)
          head = 0;
     else {
          head = ParseTerm();
          CheckAtom(head);
          Eat(6);
     }
     n = 0;
     if ((token != 10)) {
          while (TRUE) {
               n = (n + 1);
               minus = FALSE;
               if ((token == 13)) {
                    Eat(13);
                    minus = TRUE;
               }
               t = ParseTerm();
               CheckAtom(t);
               if (minus)
                    body[n - 1] = MakeNode(notsym, t, 0);
               else
                    body[n - 1] = t;
               if ((token != 9))
                    goto L3;
               Eat(9);
          }
     }
L3:
     Eat(10);
     if (errflag)
          __R__ = 0;
     else
          __R__ = MakeClause(nvars, head, body, n);
     return __R__;
}

integer ReadClause()
{
     integer c;

     integer __R__;

     do {
          hp = hmark;
          nvars = 0;
          errflag = FALSE;
          if (interacting) {
               putc('\n', output);
               fprintf(output, "# :- ");
               flush();
          }
          Scan();
          if ((token == 14))
               c = 0;
          else
               c = ParseClause(interacting);
     } while (! (((! errflag) || (token == 14))));
     __R__ = c;
     return __R__;
}

integer trhead;

void Save(v)
integer v;
{
     integer p;

     if (((v < choice) || (v >= mem[(choice + 4) - 1]))) {
          p = GloAlloc(6, 4);
          mem[(p + 2) - 1] = v;
          mem[(p + 3) - 1] = trhead;
          trhead = p;
     }
}

void Restore()
{
     integer v;

     while ((trhead != mem[(choice + 5) - 1])) {
          v = mem[(trhead + 2) - 1];
          if ((v != 0))
               mem[(v + 2) - 1] = 0;
          trhead = mem[(trhead + 3) - 1];
     }
}

void Commit()
{
     integer p;

     p = trhead;
     if ((choice == 0)) {
          putc('\n', output);
          fprintf(output, "Error: Commit");
          run = FALSE;
     }
     while (((p != 0) && (p < mem[(choice + 4) - 1]))) {
          if (((mem[(p + 2) - 1] != 0) && (! ((mem[(p + 2) - 1] <
                    choice) || (mem[(p + 2) - 1] >= mem[(choice + 4)
                    - 1])))))
               mem[(p + 2) - 1] = 0;
          p = mem[(p + 3) - 1];
     }
}

integer GloCopy(t, e)
integer t;
integer e;
{
     integer tt;
     integer i, n;

     integer __R__;

     t = Deref(t, e);
     if ((t >= gsp))
          __R__ = t;
     else {
          switch ((mem[t - 1] / 256)) {
          case 1:
               {
                    n = symtab[mem[(t + 2) - 1] - 1].arity;
                    if (((t <= hp) && (n == 0)))
                         __R__ = t;
                    else {
                         tt = GloAlloc(1, (3 + n));
                         mem[(tt + 2) - 1] = mem[(t + 2) - 1];
                         for (i = 1; i <= n; i++)
                              mem[((tt + i) + 2) - 1] =
                                        GloCopy(mem[((t + i) + 2) -
                                        1], e);
                         __R__ = tt;
                    }
               }
               break;
          case 4:
               {
                    tt = GloAlloc(4, 3);
                    mem[(tt + 2) - 1] = 0;
                    Save(t);
                    mem[(t + 2) - 1] = tt;
                    __R__ = tt;
               }
               break;
          case 2:
          case 3:
               __R__ = t;
               break;
          default:
               {
                    putc('\n', output);
                    fprintf(output, "Panic: bad tag %d in GloCopy\n",
                              (mem[t - 1] / 256));
                    halt();
               }
          }
     }
     return __R__;
}

void Share(v1, v2)
integer v1;
integer v2;
{
     if (((v1 * ((2 * ord((v1 >= gsp))) - 1)) <= (v2 * ((2 * ord((v2
               >= gsp))) - 1)))) {
          Save(v1);
          mem[(v1 + 2) - 1] = v2;
     }
     else {
          Save(v2);
          mem[(v2 + 2) - 1] = v1;
     }
}

boolean Unify(t1, e1, t2, e2)
integer t1;
integer e1;
integer t2;
integer e2;
{
     integer i;
     boolean match;

     boolean __R__;

     t1 = Deref(t1, e1);
     t2 = Deref(t2, e2);
     if ((t1 == t2))
          __R__ = TRUE;
     else if ((((mem[t1 - 1] / 256) == 4) && ((mem[t2 - 1] / 256) ==
               4))) {
          Share(t1, t2);
          __R__ = TRUE;
     }
     else if (((mem[t1 - 1] / 256) == 4)) {
          Save(t1);
          mem[(t1 + 2) - 1] = GloCopy(t2, e2);
          __R__ = TRUE;
     }
     else if (((mem[t2 - 1] / 256) == 4)) {
          Save(t2);
          mem[(t2 + 2) - 1] = GloCopy(t1, e1);
          __R__ = TRUE;
     }
     else if (((mem[t1 - 1] / 256) != (mem[t2 - 1] / 256)))
          __R__ = FALSE;
     else {
          switch ((mem[t1 - 1] / 256)) {
          case 1:
               if ((mem[(t1 + 2) - 1] != mem[(t2 + 2) - 1]))
                    __R__ = FALSE;
               else {
                    i = 1;
                    match = TRUE;
                    while ((match && (i <= symtab[mem[(t1 + 2) - 1] -
                              1].arity))) {
                         match = Unify(mem[((t1 + i) + 2) - 1], e1,
                                   mem[((t2 + i) + 2) - 1], e2);
                         i = (i + 1);
                    }
                    __R__ = match;
               }
               break;
          case 2:
               __R__ = (mem[(t1 + 2) - 1] == mem[(t2 + 2) - 1]);
               break;
          case 3:
               __R__ = (mem[(t1 + 2) - 1] == mem[(t2 + 2) - 1]);
               break;
          default:
               {
                    putc('\n', output);
                    fprintf(output, "Panic: bad tag %d in Unify\n",
                              (mem[t1 - 1] / 256));
                    halt();
               }
          }
     }
     return __R__;
}

integer Key(t, e)
integer t;
integer e;
{
     integer t0;

     integer __R__;

     if ((t == 0)) {
          putc('\n', output);
          fprintf(output, "Panic: Key\n");
          halt();
     }
     if (((mem[t - 1] / 256) != 1)) {
          putc('\n', output);
          fprintf(output, "Panic: bad tag %d in Key1\n", (mem[t - 1]
                    / 256));
          halt();
     }
     if ((symtab[mem[(t + 2) - 1] - 1].arity == 0))
          __R__ = 0;
     else {
          t0 = Deref(mem[((t + 1) + 2) - 1], e);
          switch ((mem[t0 - 1] / 256)) {
          case 1:
               __R__ = mem[(t0 + 2) - 1];
               break;
          case 2:
               __R__ = (mem[(t0 + 2) - 1] + 1);
               break;
          case 3:
               __R__ = (mem[(t0 + 2) - 1] + 1);
               break;
          case 5:
          case 4:
               __R__ = 0;
               break;
          default:
               {
                    putc('\n', output);
                    fprintf(output, "Panic: bad tag %d in Key2\n",
                              (mem[t0 - 1] / 256));
                    halt();
               }
          }
     }
     return __R__;
}

integer Search(t, e, p)
integer t;
integer e;
integer p;
{
     integer k;

     integer __R__;

     k = Key(t, e);
     if ((k != 0))
          while ((((p != 0) && (mem[(p + 1) - 1] != 0)) && (mem[(p +
                    1) - 1] != k)))
               p = mem[(p + 2) - 1];
     __R__ = p;
     return __R__;
}

boolean ok;

void PushFrame(nvars, retry)
integer nvars;
integer retry;
{
     integer f;
     integer i;

     f = LocAlloc((7 + (nvars * 3)));
     mem[f - 1] = current;
     mem[(f + 1) - 1] = goalframe;
     mem[(f + 2) - 1] = retry;
     mem[(f + 3) - 1] = choice;
     mem[(f + 4) - 1] = gsp;
     mem[(f + 5) - 1] = trhead;
     mem[(f + 6) - 1] = nvars;
     for (i = 1; i <= nvars; i++) {
          mem[((f + 7) + ((i - 1) * 3)) - 1] = ((256 * 4) + 3);
          mem[(((f + 7) + ((i - 1) * 3)) + 2) - 1] = 0;
     }
     goalframe = f;
     if ((retry != 0))
          choice = goalframe;
}

void TroStep()
{
     integer temp;
     integer oldsize, newsize, i;

     if (dflag)
          fprintf(output, "(TRO)\n");
     oldsize = (7 + (mem[(goalframe + 6) - 1] * 3));
     newsize = (7 + (mem[proc - 1] * 3));
     temp = LocAlloc(newsize);
     temp = (goalframe + newsize);
     for (i = (oldsize - 1); i >= 0; i--)
          mem[(temp + i) - 1] = mem[(goalframe + i) - 1];
     for (i = 1; i <= mem[(goalframe + 6) - 1]; i++) {
          if ((((((mem[((temp + 7) + ((i - 1) * 3)) - 1] / 256) == 4)
                    && (mem[(((temp + 7) + ((i - 1) * 3)) + 2) - 1]
                    != 0)) && (goalframe <= mem[(((temp + 7) + ((i -
                    1) * 3)) + 2) - 1])) && (mem[(((temp + 7) + ((i -
                    1) * 3)) + 2) - 1] < (goalframe + oldsize))))
               mem[(((temp + 7) + ((i - 1) * 3)) + 2) - 1] =
                         (mem[(((temp + 7) + ((i - 1) * 3)) + 2) - 1]
                         + newsize);
     }
     mem[(goalframe + 6) - 1] = mem[proc - 1];
     for (i = 1; i <= mem[(goalframe + 6) - 1]; i++) {
          mem[((goalframe + 7) + ((i - 1) * 3)) - 1] = ((256 * 4) +
                    3);
          mem[(((goalframe + 7) + ((i - 1) * 3)) + 2) - 1] = 0;
     }
     ok = Unify(call, temp, mem[(proc + 3) - 1], goalframe);
     current = (proc + 4);
     lsp = (temp - 1);
}

void Step()
{
     integer retry;

     if ((symtab[mem[(call + 2) - 1] - 1].action != 0))
          ok = DoBuiltin(symtab[mem[(call + 2) - 1] - 1].action);
     else if ((proc == 0))
          ok = FALSE;
     else {
          retry = Search(call, goalframe, mem[(proc + 2) - 1]);
          if (((((mem[(current + 1) - 1] == 0) && (choice <
                    goalframe)) && (retry == 0)) && (goalframe !=
                    base)))
               TroStep();
          else {
               PushFrame(mem[proc - 1], retry);
               ok = Unify(call, mem[(goalframe + 1) - 1], mem[(proc +
                         3) - 1], goalframe);
               current = (proc + 4);
               /* skip */;
          }
     }
}

void Unwind()
{
     while (((mem[current - 1] == 0) && (goalframe != base))) {
          if (dflag) {
               fprintf(output, "Exit: ");
               PrintTerm(mem[mem[goalframe - 1] - 1], mem[(goalframe
                         + 1) - 1], 2);
               putc('\n', output);
          }
          current = (mem[goalframe - 1] + 1);
          if ((goalframe > choice))
               lsp = (goalframe - 1);
          goalframe = mem[(goalframe + 1) - 1];
     }
}

void Backtrack()
{
     Restore();
     current = mem[choice - 1];
     goalframe = mem[(choice + 1) - 1];
     call = Deref(mem[current - 1], goalframe);
     proc = mem[(choice + 2) - 1];
     gsp = mem[(choice + 4) - 1];
     lsp = (choice - 1);
     choice = mem[(choice + 3) - 1];
     if (dflag) {
          fprintf(output, "Redo: ");
          PrintTerm(call, goalframe, 2);
          putc('\n', output);
     }
     /* skip */;
}

void Resume()
{
     while (run) {
          if (ok) {
               if ((mem[current - 1] == 0))
                    goto L2;
               call = Deref(mem[current - 1], goalframe);
               if (dflag) {
                    fprintf(output, "Call: ");
                    PrintTerm(call, goalframe, 2);
                    putc('\n', output);
               }
               if (((symtab[mem[(call + 2) - 1] - 1].proc == 0) &&
                         (symtab[mem[(call + 2) - 1] - 1].action ==
                         0))) {
                    {
                         putc('\n', output);
                         fprintf(output,
                                   "Error: call to undefined relation ");
                         run = FALSE;
                    }
                    WriteString(symtab[mem[(call + 2) - 1] - 1].name);
                    goto L2;
               }
               proc = Search(call, goalframe, symtab[mem[(call + 2) -
                         1] - 1].proc);
          }
          else {
               if ((choice <= base))
                    goto L2;
               Backtrack();
          }
          Step();
          if (ok)
               Unwind();
          if (((gsp - lsp) <= 10000))
               Collect();
     }
L2:
     /* skip */;
}

void Execute(g)
integer g;
{
     lsp = hp;
     gsp = (1000000 + 1);
     current = 0;
     goalframe = 0;
     choice = 0;
     trhead = 0;
     PushFrame(mem[g - 1], 0);
     choice = goalframe;
     base = goalframe;
     current = (g + 4);
     mem[(base + 3) - 1] = base;
     run = TRUE;
     ok = TRUE;
     do {
          Resume();
          if ((! run))
               goto L2;
          if ((! ok)) {
               putc('\n', output);
               fprintf(output, "no");
               goto L2;
          }
          ok = ShowAnswer(base);
     } while (! (ok));
     putc('\n', output);
     fprintf(output, "yes");
L2:
     /* skip */;
}

argbuf av;
integer callbody;

void GetArgs()
{
     integer i;

     for (i = 1; i <= symtab[mem[(call + 2) - 1] - 1].arity; i++)
          av[i - 1] = Deref(mem[((call + i) + 2) - 1], goalframe);
}

integer NewInt(n)
integer n;
{
     integer t;

     integer __R__;

     t = GloAlloc(2, 3);
     mem[(t + 2) - 1] = n;
     __R__ = t;
     return __R__;
}

boolean DoCut()
{
     boolean __R__;

     choice = mem[(goalframe + 3) - 1];
     lsp = ((goalframe + (7 + (mem[(goalframe + 6) - 1] * 3))) - 1);
     Commit();
     current = (current + 1);
     __R__ = TRUE;
     return __R__;
}

boolean DoCall()
{
     boolean __R__;

     GetArgs();
     if ((! ((mem[av[1 - 1] - 1] / 256) == 1))) {
          {
               putc('\n', output);
               fprintf(output, "Error: bad argument to call/1");
               run = FALSE;
          }
          __R__ = FALSE;
     }
     else {
          PushFrame(1, 0);
          mem[(((goalframe + 7) + ((1 - 1) * 3)) + 2) - 1] =
                    GloCopy(av[1 - 1], mem[(goalframe + 1) - 1]);
          current = callbody;
          __R__ = TRUE;
     }
     return __R__;
}

boolean DoNot()
{
     integer savebase;

     boolean __R__;

     GetArgs();
     if ((! ((mem[av[1 - 1] - 1] / 256) == 1))) {
          {
               putc('\n', output);
               fprintf(output, "Error: bad argument to call/1");
               run = FALSE;
          }
          __R__ = FALSE;
     }
     else {
          PushFrame(1, 0);
          savebase = base;
          base = goalframe;
          choice = goalframe;
          mem[(((goalframe + 7) + ((1 - 1) * 3)) + 2) - 1] =
                    GloCopy(av[1 - 1], mem[(goalframe + 1) - 1]);
          current = callbody;
          ok = TRUE;
          Resume();
          choice = mem[(base + 3) - 1];
          goalframe = mem[(base + 1) - 1];
          if ((! ok)) {
               current = (mem[base - 1] + 1);
               __R__ = TRUE;
          }
          else {
               Commit();
               __R__ = FALSE;
          }
          lsp = (base - 1);
          base = savebase;
     }
     return __R__;
}

boolean DoPlus()
{
     boolean result;

     boolean __R__;

     GetArgs();
     result = FALSE;
     if ((((mem[av[1 - 1] - 1] / 256) == 2) && ((mem[av[2 - 1] - 1] /
               256) == 2)))
          result = Unify(av[3 - 1], goalframe, NewInt((mem[(av[1 - 1]
                    + 2) - 1] + mem[(av[2 - 1] + 2) - 1])), 0);
     else if ((((mem[av[1 - 1] - 1] / 256) == 2) && ((mem[av[3 - 1] -
               1] / 256) == 2))) {
          if ((mem[(av[1 - 1] + 2) - 1] <= mem[(av[3 - 1] + 2) - 1]))
               result = Unify(av[2 - 1], goalframe, NewInt((mem[(av[3
                         - 1] + 2) - 1] - mem[(av[1 - 1] + 2) - 1])),
                         0);
     }
     else if ((((mem[av[2 - 1] - 1] / 256) == 2) && ((mem[av[3 - 1] -
               1] / 256) == 2))) {
          if ((mem[(av[2 - 1] + 2) - 1] <= mem[(av[3 - 1] + 2) - 1]))
               result = Unify(av[1 - 1], goalframe, NewInt((mem[(av[3
                         - 1] + 2) - 1] - mem[(av[2 - 1] + 2) - 1])),
                         0);
     }
     else {
          putc('\n', output);
          fprintf(output,
                    "Error: plus/3 needs at least two integers");
          run = FALSE;
     }
     current = (current + 1);
     __R__ = result;
     return __R__;
}

boolean DoTimes()
{
     boolean result;

     boolean __R__;

     GetArgs();
     result = FALSE;
     if ((((mem[av[1 - 1] - 1] / 256) == 2) && ((mem[av[2 - 1] - 1] /
               256) == 2)))
          result = Unify(av[3 - 1], goalframe, NewInt((mem[(av[1 - 1]
                    + 2) - 1] * mem[(av[2 - 1] + 2) - 1])), 0);
     else if ((((mem[av[1 - 1] - 1] / 256) == 2) && ((mem[av[3 - 1] -
               1] / 256) == 2))) {
          if ((mem[(av[1 - 1] + 2) - 1] != 0))
               if (((mem[(av[3 - 1] + 2) - 1] % mem[(av[1 - 1] + 2) -
                         1]) == 0))
                    result = Unify(av[2 - 1], goalframe,
                              NewInt((mem[(av[3 - 1] + 2) - 1] /
                              mem[(av[1 - 1] + 2) - 1])), 0);
     }
     else if ((((mem[av[2 - 1] - 1] / 256) == 2) && ((mem[av[3 - 1] -
               1] / 256) == 2))) {
          if ((mem[(av[2 - 1] + 2) - 1] != 0))
               if (((mem[(av[3 - 1] + 2) - 1] % mem[(av[2 - 1] + 2) -
                         1]) == 0))
                    result = Unify(av[1 - 1], goalframe,
                              NewInt((mem[(av[3 - 1] + 2) - 1] /
                              mem[(av[2 - 1] + 2) - 1])), 0);
     }
     else {
          putc('\n', output);
          fprintf(output,
                    "Error: times/3 needs at least two integers");
          run = FALSE;
     }
     current = (current + 1);
     __R__ = result;
     return __R__;
}

boolean DoEqual()
{
     boolean __R__;

     GetArgs();
     current = (current + 1);
     __R__ = Unify(av[1 - 1], goalframe, av[2 - 1], goalframe);
     return __R__;
}

boolean DoInteger()
{
     boolean __R__;

     GetArgs();
     current = (current + 1);
     __R__ = ((mem[av[1 - 1] - 1] / 256) == 2);
     return __R__;
}

boolean DoChar()
{
     boolean __R__;

     GetArgs();
     current = (current + 1);
     __R__ = ((mem[av[1 - 1] - 1] / 256) == 3);
     return __R__;
}

boolean DoPrint()
{
     boolean __R__;

     GetArgs();
     PrintTerm(av[1 - 1], goalframe, 2);
     current = (current + 1);
     __R__ = TRUE;
     return __R__;
}

boolean DoNl()
{
     boolean __R__;

     putc('\n', output);
     current = (current + 1);
     __R__ = TRUE;
     return __R__;
}

boolean DoBuiltin(action)
integer action;
{
     boolean __R__;

     switch (action) {
     case 1:
          __R__ = DoCut();
          break;
     case 2:
          __R__ = DoCall();
          break;
     case 3:
          __R__ = DoPlus();
          break;
     case 4:
          __R__ = DoTimes();
          break;
     case 5:
          __R__ = DoInteger();
          break;
     case 6:
          __R__ = DoChar();
          break;
     case 7:
          __R__ = DoNot();
          break;
     case 8:
          __R__ = DoEqual();
          break;
     case 9:
          __R__ = FALSE;
          break;
     case 10:
          __R__ = DoPrint();
          break;
     case 11:
          __R__ = DoNl();
          break;
     default:
          {
               putc('\n', output);
               fprintf(output, "Panic: bad tag %d in DoBuiltin\n",
                         action);
               halt();
          }
     }
     return __R__;
}

integer shift;

void Visit(t)
integer t;
{
     integer i, n;

     while ((t != 0)) {
          if (((! (t >= gsp)) || ((mem[t - 1] % 256) >= 128)))
               goto L2;
          mem[t - 1] = (mem[t - 1] + 128);
          switch ((mem[t - 1] / 256)) {
          case 1:
               {
                    n = symtab[mem[(t + 2) - 1] - 1].arity;
                    if ((n == 0))
                         goto L2;
                    for (i = 1; i <= (n - 1); i++)
                         Visit(mem[((t + i) + 2) - 1]);
                    t = mem[((t + n) + 2) - 1];
               }
               break;
          case 4:
               t = mem[(t + 2) - 1];
               break;
          case 2:
          case 3:
               goto L2;
               break;
          default:
               {
                    putc('\n', output);
                    fprintf(output, "Panic: bad tag %d in Visit\n",
                              (mem[t - 1] / 256));
                    halt();
               }
          }
     }
L2:
     /* skip */;
}

void MarkStack()
{
     integer f;
     integer i;

     f = (hp + 1);
     while ((f <= lsp)) {
          for (i = 1; i <= mem[(f + 6) - 1]; i++)
               if (((mem[((f + 7) + ((i - 1) * 3)) - 1] / 256) == 4))
                    Visit(mem[(((f + 7) + ((i - 1) * 3)) + 2) - 1]);
          f = (f + (7 + (mem[(f + 6) - 1] * 3)));
     }
}

void CullTrail(p)
integer (*p);
{
     while ((*p != 0)) {
          if ((mem[(*p + 2) - 1] != 0))
               if (((! (mem[(*p + 2) - 1] >= gsp)) || ((mem[mem[(*p +
                         2) - 1] - 1] % 256) >= 128)))
                    goto L2;
          *p = mem[(*p + 3) - 1];
     }
L2:
     /* skip */;
}

void MarkTrail()
{
     integer p;

     CullTrail(&trhead);
     p = trhead;
     while ((p != 0)) {
          mem[p - 1] = (mem[p - 1] + 128);
          CullTrail(&mem[(p + 3) - 1]);
          p = mem[(p + 3) - 1];
     }
}

void Relocate()
{
     integer p;
     integer step;

     shift = 0;
     p = gsp;
     while ((p <= 1000000)) {
          step = (mem[p - 1] % 128);
          mem[(p + 1) - 1] = shift;
          if ((! ((mem[p - 1] % 256) >= 128)))
               shift = (shift + step);
          p = (p + step);
     }
}

void AdjustPointer(p)
integer (*p);
{
     if (((*p != 0) && (*p >= gsp))) {
          if ((! ((mem[*p - 1] % 256) >= 128))) {
               putc('\n', output);
               fprintf(output,
                         "Panic: adjusting pointer to unmarked block\n");
               halt();
          }
          *p = ((*p + shift) - mem[(*p + 1) - 1]);
     }
}

void AdjustStack()
{
     integer f;
     integer i;
     integer q;

     f = (hp + 1);
     while ((f <= lsp)) {
          q = mem[(f + 4) - 1];
          while ((q <= 1000000)) {
               if (((mem[q - 1] % 256) >= 128))
                    goto L1;
               q = (q + (mem[q - 1] % 128));
          }
     L1:
          if ((q <= 1000000))
               AdjustPointer(&q);
          mem[(f + 4) - 1] = q;
          q = mem[(f + 5) - 1];
          while ((q != 0)) {
               if (((mem[q - 1] % 256) >= 128))
                    goto L4;
               q = mem[(q + 3) - 1];
          }
     L4:
          AdjustPointer(&q);
          mem[(f + 5) - 1] = q;
          for (i = 1; i <= mem[(f + 6) - 1]; i++)
               if (((mem[((f + 7) + ((i - 1) * 3)) - 1] / 256) == 4))
                    AdjustPointer(&mem[(((f + 7) + ((i - 1) * 3)) +
                              2) - 1]);
          f = (f + (7 + (mem[(f + 6) - 1] * 3)));
          /* skip */;
     }
}

void AdjustInternal()
{
     integer p, i;

     p = gsp;
     while ((p <= 1000000)) {
          if (((mem[p - 1] % 256) >= 128)) {
               switch ((mem[p - 1] / 256)) {
               case 1:
                    for (i = 1; i <= symtab[mem[(p + 2) - 1] -
                              1].arity; i++)
                         AdjustPointer(&mem[((p + i) + 2) - 1]);
                    break;
               case 4:
                    AdjustPointer(&mem[(p + 2) - 1]);
                    break;
               case 6:
                    {
                         AdjustPointer(&mem[(p + 2) - 1]);
                         AdjustPointer(&mem[(p + 3) - 1]);
                    }
                    break;
               case 2:
               case 3:
                    /* skip */;
                    break;
               default:
                    {
                         putc('\n', output);
                         fprintf(output,
                                   "Panic: bad tag %d in Adjust\n",
                                   (mem[p - 1] / 256));
                         halt();
                    }
               }
          }
          p = (p + (mem[p - 1] % 128));
     }
}

void Compact()
{
     integer p, q, step, i;

     p = gsp;
     q = gsp;
     while ((p <= 1000000)) {
          step = (mem[p - 1] % 128);
          if (((mem[p - 1] % 256) >= 128)) {
               mem[p - 1] = (mem[p - 1] - 128);
               for (i = 0; i <= (step - 1); i++)
                    mem[(q + i) - 1] = mem[(p + i) - 1];
               q = (q + step);
          }
          p = (p + step);
     }
     gsp = (gsp + shift);
     for (i = 1000000; i >= gsp; i--)
          mem[i - 1] = mem[(i - shift) - 1];
     /* skip */;
}

void Collect()
{
     fprintf(output, "[gc");
     flush();
     Visit(call);
     MarkStack();
     MarkTrail();
     Relocate();
     AdjustPointer(&call);
     AdjustPointer(&trhead);
     AdjustStack();
     AdjustInternal();
     Compact();
     putc(']', output);
     flush();
     if (((gsp - lsp) <= 50000)) {
          putc('\n', output);
          fprintf(output, "Error: out of memory space");
          run = FALSE;
     }
}

void Initialize()
{
     integer i;
     integer p;

     dflag = FALSE;
     errcount = 0;
     pbchar = chr(127);
     charptr = 0;
     hp = 0;
     InitSymbols();
     for (i = 1; i <= 63; i++) {
          p = HeapAlloc(3);
          mem[p - 1] = ((256 * 5) + 3);
          mem[(p + 2) - 1] = i;
          refnode[i - 1] = p;
     }
     callbody = HeapAlloc(2);
     mem[callbody - 1] = MakeRef(1);
     mem[(callbody + 1) - 1] = 0;
}

void ReadFile()
{
     integer c;

     lineno = 1;
     do {
          hmark = hp;
          c = ReadClause();
          if ((c != 0)) {
               if (dflag)
                    PrintClause(c);
               if ((mem[(c + 3) - 1] != 0))
                    AddClause(c);
               else {
                    if (interacting) {
                         pbchar = chr(127);
                         skipln(input);
                    }
                    Execute(c);
                    putc('\n', output);
                    hp = hmark;
               }
          }
     } while (! ((c == 0)));
}

void ReadProgram()
{
     integer i0, i;
     tempstring arg;

     i0 = 1;
     if ((argc() > 1)) {
          argv(1, arg);
          if ((((arg[1 - 1] == '-') && (arg[2 - 1] == 'd')) && (arg[3
                    - 1] == chr(0)))) {
               dflag = TRUE;
               i0 = (i0 + 1);
          }
     }
     for (i = i0; i <= (argc() - 1); i++) {
          argv(i, arg);
          filename = SaveString(arg);
          if ((! openin(infile, arg))) {
               fprintf(output, "Can\'t read ");
               WriteString(filename);
               putc('\n', output);
               halt();
          }
          fprintf(output, "Reading ");
          WriteString(filename);
          putc('\n', output);
          ReadFile();
          closein(infile);
          if ((errcount > 0))
               halt();
     }
}

void p_main()
{
     fprintf(output, "Welcome to picoProlog\n");
     Initialize();
     interacting = FALSE;
     ReadProgram();
     interacting = TRUE;
     lineno = 1;
     ReadFile();
     putc('\n', output);
L999:
     /* skip */;
}
