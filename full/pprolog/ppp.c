/* Program 'ppp' */

#include "ptclib.i"

typedef char tempstring[256];

typedef char charbuf[4096];

integer ndefs;
struct {
     integer name;
     integer body;
     integer next;
} deftab[256];
integer hashtab[127];
integer defhwm;
charbuf defbuf;
integer nargs;
integer arghwm;
integer arg[128];
charbuf argbuf;
integer pbp;
charbuf pshbuf;
integer DEFINE, IFDEF;
char ENDSTR, TAB, ENDLINE, ENDFILE;
tempstring emptystr;

boolean isletter(c)
char c;
{
     boolean __R__;

     __R__ = ((((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <=
               'z'))) || (c == '_'));
     return __R__;
}

boolean isalphanum(c)
char c;
{
     boolean __R__;

     __R__ = (isletter(c) || ((c >= '0') && (c <= '9')));
     return __R__;
}

char getchar()
{
     char ch;

     char __R__;

     if (eof(input))
          __R__ = ENDFILE;
     else if (eoln(input)) {
          skipln(input);
          __R__ = ENDLINE;
     }
     else {
          ch = getc(input);
          if ((ch == '&'))
               ch = getc(input);
          __R__ = ch;
     }
     return __R__;
}

void outchar(ch)
char ch;
{
     if ((ch == ENDLINE))
          putc('\n', output);
     else if ((ch != ENDFILE))
          putc(ch, output);
}

void putstring(s)
tempstring s;
{
     integer i;

     i = 1;
     while ((s[i - 1] != ENDSTR)) {
          putc(s[i - 1], output);
          i = (i + 1);
     }
}

integer length(s)
tempstring s;
{
     integer i;

     integer __R__;

     i = 0;
     while ((s[(i + 1) - 1] != ENDSTR))
          i = (i + 1);
     __R__ = i;
     return __R__;
}

boolean eqstr(s, t)
tempstring s;
tempstring t;
{
     integer i;

     boolean __R__;

     i = 1;
     while (((s[i - 1] != ENDSTR) && (s[i - 1] == t[i - 1])))
          i = (i + 1);
     __R__ = (s[i - 1] == t[i - 1]);
     return __R__;
}

void loadstr(s, b, k)
tempstring s;
charbuf b;
integer k;
{
     integer i;

     i = 1;
     do {
          s[i - 1] = b[k - 1];
          i = (i + 1);
          k = (k + 1);
     } while (! ((s[(i - 1) - 1] == ENDSTR)));
     /* skip */;
}

void savestr(s, b, k)
tempstring s;
charbuf b;
integer k;
{
     integer i;

     i = 1;
     do {
          b[k - 1] = s[i - 1];
          i = (i + 1);
          k = (k + 1);
     } while (! ((b[(k - 1) - 1] == ENDSTR)));
     /* skip */;
}

void pbchar(ch)
char ch;
{
     if ((pbp >= 4096)) {
          fprintf(output, "Too many chars pushed back\n");
          halt();
     }
     pbp = (pbp + 1);
     pshbuf[pbp - 1] = ch;
}

void pbstring(s)
tempstring s;
{
     integer i;

     for (i = length(s); i >= 1; i--)
          pbchar(s[i - 1]);
}

char getpbc()
{
     char __R__;

     if ((pbp == 0))
          __R__ = getchar();
     else {
          __R__ = pshbuf[pbp - 1];
          pbp = (pbp - 1);
     }
     return __R__;
}

void skipws()
{
     char ch;

     do {
          ch = getpbc();
     } while (! ((((ch != ' ') && (ch != TAB)) && (ch != ENDLINE))));
     pbchar(ch);
}

integer hash(s)
tempstring s;
{
     integer h, i;

     integer __R__;

     h = 0;
     for (i = 1; i <= length(s); i++)
          h = (((5 * h) + ord(s[i - 1])) % 127);
     __R__ = (h + 1);
     return __R__;
}

integer lookup(key)
tempstring key;
{
     integer p;
     boolean found;
     tempstring name;

     integer __R__;

     p = hashtab[hash(key) - 1];
     found = FALSE;
     while (((p != 0) && (! found))) {
          loadstr(name, defbuf, deftab[p - 1].name);
          if (eqstr(key, name))
               found = TRUE;
          else
               p = deftab[p - 1].next;
     }
     __R__ = p;
     return __R__;
}

void install(name, body)
tempstring name;
tempstring body;
{
     integer h;

     h = hash(name);
     ndefs = (ndefs + 1);
     deftab[ndefs - 1].name = defhwm;
     savestr(name, defbuf, defhwm);
     defhwm = ((defhwm + length(name)) + 1);
     deftab[ndefs - 1].body = defhwm;
     savestr(body, defbuf, defhwm);
     defhwm = ((defhwm + length(body)) + 1);
     deftab[ndefs - 1].next = hashtab[h - 1];
     hashtab[h - 1] = ndefs;
}

void getargs()
{
     char ch;
     integer level;

     nargs = 0;
     arghwm = 1;
     ch = getpbc();
     if ((ch != '('))
          pbchar(ch);
     else {
          do {
               nargs = (nargs + 1);
               arg[nargs - 1] = arghwm;
               skipws();
               level = 0;
               ch = getpbc();
               while ((((ch != ',') && (ch != ')')) || (level > 0))) {
                    if ((ch == ENDFILE)) {
                         fprintf(output,
                                   "End of file reached in macro call\n");
                         halt();
                    }
                    argbuf[arghwm - 1] = ch;
                    arghwm = (arghwm + 1);
                    if ((ch == '('))
                         level = (level + 1);
                    else if ((ch == ')'))
                         level = (level - 1);
                    ch = getpbc();
               }
               argbuf[arghwm - 1] = ENDSTR;
               arghwm = (arghwm + 1);
          } while (! ((ch == ')')));
     }
}

void expand(def)
integer def;
{
     integer body, i, j, k;
     tempstring s, t;

     if ((deftab[def - 1].name == DEFINE)) {
          if ((nargs >= 1)) {
               loadstr(s, argbuf, arg[1 - 1]);
               if ((nargs == 1))
                    install(s, emptystr);
               else {
                    loadstr(t, argbuf, arg[2 - 1]);
                    install(s, t);
               }
          }
     }
     else if ((deftab[def - 1].name == IFDEF)) {
          if ((nargs >= 2)) {
               loadstr(s, argbuf, arg[1 - 1]);
               if ((lookup(s) != 0)) {
                    loadstr(t, argbuf, arg[2 - 1]);
                    pbstring(t);
               }
               else if ((nargs >= 3)) {
                    loadstr(t, argbuf, arg[3 - 1]);
                    pbstring(t);
               }
          }
     }
     else {
          body = deftab[def - 1].body;
          i = (body - 1);
          while ((defbuf[(i + 1) - 1] != ENDSTR))
               i = (i + 1);
          while ((i >= body)) {
               if ((defbuf[(i - 1) - 1] != '$'))
                    pbchar(defbuf[i - 1]);
               else if ((defbuf[i - 1] == '$'))
                    pbchar('$');
               else {
                    k = (ord(defbuf[i - 1]) - ord('0'));
                    if (((k > 0) && (k <= nargs))) {
                         loadstr(s, argbuf, arg[k - 1]);
                         pbstring(s);
                    }
                    else if (((k == 0) && (nargs > 0))) {
                         loadstr(s, argbuf, arg[nargs - 1]);
                         pbstring(s);
                         for (j = (nargs - 1); j >= 1; j--) {
                              pbchar(' ');
                              pbchar(',');
                              loadstr(s, argbuf, arg[j - 1]);
                              pbstring(s);
                         }
                    }
                    i = (i - 1);
               }
               i = (i - 1);
          }
     }
}

void copyquote(open, close)
char open;
char close;
{
     char ch;

     outchar(open);
     do {
          ch = getpbc();
          outchar(ch);
     } while (! (((ch == close) || (ch == ENDFILE))));
}

void scan()
{
     char ch;
     tempstring name;
     integer def;
     integer i;

     ch = getpbc();
     while ((ch != ENDFILE)) {
          if (isletter(ch)) {
               i = 1;
               while (isalphanum(ch)) {
                    name[i - 1] = ch;
                    i = (i + 1);
                    ch = getpbc();
               }
               pbchar(ch);
               name[i - 1] = ENDSTR;
               def = lookup(name);
               if ((def == 0))
                    putstring(name);
               else {
                    getargs();
                    expand(def);
               }
          }
          else if ((ch == '{'))
               copyquote('{', '}');
          else if ((ch == '\''))
               copyquote('\'', '\'');
          else
               outchar(ch);
          ch = getpbc();
     }
}

void init()
{
     integer i;
     tempstring s;

     ENDSTR = chr(0);
     TAB = chr(9);
     ENDLINE = chr(10);
     ENDFILE = chr(127);
     ndefs = 0;
     defhwm = 1;
     pbp = 0;
     emptystr[1 - 1] = ENDSTR;
     for (i = 1; i <= 127; i++)
          hashtab[i - 1] = 0;
     s[1 - 1] = 'd';
     s[2 - 1] = 'e';
     s[3 - 1] = 'f';
     s[4 - 1] = 'i';
     s[5 - 1] = 'n';
     s[6 - 1] = 'e';
     s[7 - 1] = ENDSTR;
     install(s, emptystr);
     DEFINE = deftab[ndefs - 1].name;
     s[1 - 1] = 'i';
     s[2 - 1] = 'f';
     s[3 - 1] = 'd';
     s[4 - 1] = 'e';
     s[5 - 1] = 'f';
     s[6 - 1] = ENDSTR;
     install(s, emptystr);
     IFDEF = deftab[ndefs - 1].name;
}

void p_main()
{
     init();
     scan();
     /* skip */;
}
