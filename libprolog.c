/** @file       libprolog.c
 *  @brief      A small prolog interpreter library.
 *  @author     Richard James Howe.
 *  @copyright  Copyright 2015 Richard James Howe.
 *  @license    LGPL v2.1 or later version
 *  @email      howe.r.j.89@gmail.com 
 *
 *  Grammar of this prolog: 
 *
 *  Program   ::= QueryOrRule | QueryOrRule Program
 *
 *  QueryOrRule ::= Query | Rule
 *  Query       ::= ?- Term .
 *  Rule        ::= Term . | Term :- Terms . 
 *  Terms       ::= Term   | Term , Terms
 *  Term        ::= Number | Variable | AtomName | AtomName(Terms)
 *                  | [] | [Terms] | [Terms | Term]
 *                  | print(Term) | nl | eq(Term , Term)
 *                  | if(Term , Term , Term) | or(Term , Term ) 
 *                  | not(Term) | call(Term) | once(Term)
 *  Number      ::= Digit | Digit Number
 *  AtomName    ::= LowerCase | LowerCase NameChars
 *  Variable    ::= UpperCase | UpperCase NameChars
 *  NameChars   ::= NameChar  | NameChar  NameChars
 *  NameChar    ::= a | ... | z | A | ... | Z | Digit 
 *  Digit       ::= 0 | ... | 9
 **/
#include "libprolog.h"
#include <assert.h>
#include <ctype.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *errorfmt = "( error \"%s%s\" \"%s:%u\" %zu )\n";
static const char *initprg  = " ";

#define PWARN(P,M) fprintf(stderr,errorfmt,(P),(M),__FILE__,__LINE__,o->lc)
#define WARN(M)    PWARN("",(M))
#define FAIL(O,M)  do { WARN((M)); longjmp((O)->exception, 1); } while(0);
#define ID_MAX     (256u)

struct prolog_obj {         
        FILE *in, *out;    /*file input (if not using string input), output*/
        int ch, sym, ival; /*current char, current symbol, lexed value*/
        char id[ID_MAX];   /*lexed identifier*/
        uint8_t *sin;      /*string input pointer*/
        size_t slen, sidx; /*string input length and index into buffer*/
        size_t lc;         /*number of new lines encountered*/
        unsigned invalid :1, stringin :1; /*invalidate obj? string input? */
        jmp_buf exception; /*jump here on exception*/
};

static int ogetc(prolog_obj_t *o)
{       if(o->stringin) return o->sidx >= o->slen ? EOF : o->sin[o->sidx++];
        else return fgetc(o->in);
} /*get a char from string-in or a file*/

static FILE *fopenj(const char *name, char *mode, jmp_buf *go)
{       FILE *file = fopen(name, mode);
        if(!file) perror(name), longjmp(*go, 1);
        return file;
}

static int isnum(const char *s) /*needs fixing; proper octal and hex*/
{ s += *s == '-' ? 1 : 0; return s[strspn(s,"0123456789")] == '\0'; }

static int comment(prolog_obj_t *o)
{       int c; 
        while(((c = ogetc(o)) > 0) && (c != '\n'));
        return c;
} /*process a comment line from input*/

/* -- Lexer ---------------------------------------------------------------- */

enum { 
        /*each of these symbols has an entry in *words[]*/
        PRINT_SYM, NL_SYM,  EQ_SYM,   IF_SYM, 
        OR_SYM,    NOT_SYM, CALL_SYM, ONCE_SYM,
        /*theses ones do not*/
        LPAR,   RPAR,   LSPAR, RSPAR, 
        PERIOD, COMMA,  QUERY, ASSIGN, 
        ID,     VARLEX, INT,   EOI 
     };
static char *words[] = {"print","nl","eq","if","or","not","call","once",NULL};

static void next_ch(prolog_obj_t *o) { o->ch = ogetc(o); }

static void next_sym(prolog_obj_t *o)
{ again: switch(o->ch)
        { case ' ': case '\t': next_ch(o); goto again;
          case '\n': o->lc++; next_ch(o); goto again;
          case '\0': case EOF: o->sym = EOI; break;
          case '#': comment(o); next_ch(o); goto again;
          case '(': o->sym = LPAR;    next_ch(o); break;
          case ')': o->sym = RPAR;    next_ch(o); break;
          case '[': o->sym = LSPAR;   next_ch(o); break;
          case ']': o->sym = RSPAR;   next_ch(o); break;
          case '.': o->sym = PERIOD;  next_ch(o); break;
          case ',': o->sym = COMMA;   next_ch(o); break;
          case '?': next_ch(o); 
                    if(o->ch != '-') FAIL(o,"expected '-'"); 
                    o->sym = QUERY;
                    next_ch(o);
                    break;
          case ':': next_ch(o); 
                    if(o->ch != '-') FAIL(o, "expected '-'"); 
                    o->sym = ASSIGN;
                    next_ch(o);
                    break;
          default:
                if(isdigit(o->ch)) { /*integer*/
                        o->ival = 0;
                        while(isdigit(o->ch)) {
                                o->ival = o->ival*10 + (o->ch - '0'); 
                                next_ch(o);
                        }
                        o->sym = INT;
                } else if(isalpha(o->ch)) { /*variable, atom or keyword*/
                        size_t i = 0;
                        while(isalnum(o->ch)) {
                                if(i > ID_MAX) FAIL(o, "identifier too longer");
                                o->id[i++] = o->ch;
                                next_ch(o);
                        }
                        o->id[i] = '\0';
                        if(isupper(o->id[0])) { /*variable*/
                                o->sym = VARLEX;
                                break;
                        } else { /*id or keyword*/
                                o->sym = 0;
                                while(words[o->sym] && strcmp(words[o->sym], o->id))
                                        o->sym++;
                                if(!words[o->sym]) /*id*/
                                        o->sym = ID;
                                break;
                        }
                } else {
                        FAIL(o, "invalid char"); 
                }
                break;
        }
}

/* -- Parser --------------------------------------------------------------- */

enum { TERMS, TERM, RULE, PQUERY, QUERYORRULE, PROG };

#define PDEBUG(NUM,NAME) fprintf(stderr, "( parse %d %s )\n", (NUM), (NAME));

struct node { int type; struct node *o1, *o2, *o3; int ival; };
typedef struct node node;

static node *term(prolog_obj_t *o);

static node *new_node(prolog_obj_t *o, int type)
{       node *x = calloc(1, sizeof(node));
        if(!x) FAIL(o, "allocation failed"); 
        x->type = type;
        return x;
}


static node *terms(prolog_obj_t *o)
{       node *x = new_node(o, TERMS), *t;
        PDEBUG(x->type, "terms");
        t = x;
        while(o->sym != EOI && o->sym != PERIOD){
                t->o2 = term(o);
                t = t->o2;
                next_sym(o);
        }
        return x;
}

static node *term(prolog_obj_t *o)
{       node *x = new_node(o, TERM);
        PDEBUG(x->type, "term");
        next_sym(o);
        return x;
}

static node *rule(prolog_obj_t *o)
{       node *x = new_node(o, RULE);
        PDEBUG(x->type, "rule");
        if(o->sym == PERIOD)
                return x;
        x->o1 = term(o);
        if(o->sym == PERIOD)
                return x;
        if(o->sym == ASSIGN) { 
                next_sym(o);
                x->o2 = terms(o);
                if(o->sym != PERIOD) 
                        FAIL(o, "expected '.'");
                return x;
        }
        FAIL(o, "expected ':-' or '.'");
        return x;
}

static node *pquery(prolog_obj_t *o)
{       node *x = new_node(o, PQUERY);
        PDEBUG(x->type, "pquery");
        return x;
}

static node *queryorrule(prolog_obj_t *o)
{       node *x = new_node(o, QUERYORRULE);
        PDEBUG(x->type, "queryorrule");
        while(o->sym != EOI){
                if(o->sym == QUERY) {
                        next_sym(o);
                        x->o1 = pquery(o);
                } else {
                        x->o1 = rule(o);
                }
                next_sym(o);
        }
        return x;
}

static node *program(prolog_obj_t *o)
{       node *x = new_node(o, PROG);
        PDEBUG(x->type, "program");
        next_sym(o); 
        x->o1 = queryorrule(o); 
        if(o->sym != EOI)
                FAIL(o, "expected EOI");
        return x;
}

/* -- Execution ------------------------------------------------------------ */
/* -- Interface ------------------------------------------------------------ */

void prolog_seti(prolog_obj_t *o, FILE *in)  
{ assert(o && in);  o->stringin = 0; o->lc = 1; o->in  = in;  }

void prolog_seto(prolog_obj_t *o, FILE *out) 
{ assert(o && out); o->stringin = 0; o->out = out; }

void prolog_sets(prolog_obj_t *o, const char *s)
{       assert(o && s);
        o->sidx = 0;             /*sidx == current character in string*/
        o->slen = strlen(s) + 1; /*slen == actual string len*/
        o->stringin = 1;         /*read from string not a file handle*/
        o->lc = 1;               /*linecount*/
        o->sin = (uint8_t *)s;
}

int prolog_eval(prolog_obj_t *o, const char *s)
{ assert(o && s); prolog_sets(o,s); return prolog_run(o);}

prolog_obj_t *prolog_init(FILE *in, FILE *out)
{
        prolog_obj_t *o;

        if(!in || !out || !(o = calloc(1, sizeof(*o)))) return NULL;
        o->out = out;
        o->ch = ' ';
        if(prolog_eval(o, initprg) < 0) return NULL; /*define words*/
        o->ch = ' ';
        o->sym = PERIOD;
        prolog_seti(o, in);                  /*set up input after out eval*/
        return o;
}

int prolog_run(prolog_obj_t *o)
{       
        if(!o || o->invalid) return WARN("invalid obj"), -1;
        if(setjmp(o->exception)) return -(o->invalid = 1); /*setup handler*/

        program(o);
        /*
        while(o->sym != EOI)
                next_sym(o), printf("%d\n", o->sym);*/

        return 0;
}

int main_prolog(int argc, char **argv)  /*options: ./prolog (file)* */
{       int rval = 0, c;                /*return value, temp char*/
        FILE *in = NULL;                /*current input file*/
        jmp_buf go;                     /*exception handler state*/
        prolog_obj_t *o = NULL;         /*our prolog environment*/
        if(!(o = prolog_init(stdin, stdout))) return -1; /*setup env*/
        if(setjmp(go)) return free(o), -1;     /*setup exception handler*/
        if(argc > 1)
                while(++argv, --argc) {
                        prolog_seti(o, in = fopenj(argv[0], "rb", &go));
/*shebang line '#!' */  if((c=fgetc(in)) == '#') comment(o); /*special case*/
                        else if (c == EOF){ fclose(in), in = NULL; continue; }
                        else ungetc(c,in);
                        if((rval = prolog_run(o)) < 0) goto END;
                        fclose(in), in = NULL;
                }
        else rval = prolog_run(o); /*read from defaults, stdin*/
END:    if(in && (in != stdin)) fclose(in), in = NULL;
        free(o), o = NULL;
        return rval;
}
