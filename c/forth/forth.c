#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "forth.h"

#define I tobj->I
#define t tobj->t
#define w tobj->w
#define f tobj->f

/*static function prototypes*/
static void compile_word(third_obj_t * tobj, int code, int flag, char *str);

static void compile_word(third_obj_t * tobj, int code, int flag, char *str)
{
        char *s = tobj->s;
        int *m = tobj->m;
        m[m[0]++] = tobj->L;
        tobj->L = *m - 1;
        m[m[0]++] = t;
        m[m[0]++] = code;
        if (flag)
                strcpy(s + t, str);
        else
                fscanf(tobj->input, "%s", s + t);
        t += strlen(s + t) + 1;
        return;
}

int init_third(third_obj_t * tobj, FILE * input, FILE * output)
{
        int *m = tobj->m;
        tobj->input = input;
        tobj->output = output;

        m[0] = 32;
        tobj->L = 1;
        I = 0;
        tobj->S = tobj->T;
        t = 64;
        w = 0;
        f = 0;
        tobj->x = 0;

        compile_word(tobj, DEFINE, true, ":");
        compile_word(tobj, IMMEDIATE, true, "immediate");
        compile_word(tobj, COMPILE, true, "read");
        w = *m;
        m[m[0]++] = READ;
        m[m[0]++] = RUN;
        I = *m;
        m[m[0]++] = w;
        m[m[0]++] = I - 1;
        w = LOAD;

        compile_word(tobj, COMPILE, true, "@");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, "!");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, "-");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, "+");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, "*");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, "/");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, "<0");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, "exit");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, "emit");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, "key");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, "r>");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, ">r");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, "jmp");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, "jmpz");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, ".");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, "'");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, ",");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, "not");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, "=");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, "swap");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, "dup");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, "drop");
        m[m[0]++] = w++;
        compile_word(tobj, COMPILE, true, "tail");
        m[m[0]++] = w++;

        m[1] = *m;
        *m += 512;
        return I;
}

void run_third(third_obj_t * tobj)
{
        char *s = tobj->s;
        int *m = tobj->m, *x = &tobj->x, *S = tobj->S;
        while (true) {
                *x = m[I++];
 INNER:
                switch (m[(*x)++]) {
                case PUSH:
                        *++S = f;
                        f = m[I++];
                        break;
                case COMPILE:
                        m[m[0]++] = *x;
                        break;
                case RUN:
                        m[++m[1]] = I;
                        I = *x;
                        break;
                case DEFINE:
                        m[8] = 1;
                        compile_word(tobj, COMPILE, false, NULL);
                        m[m[0]++] = RUN;
                        break;
                case IMMEDIATE:
                        *m -= 2;
                        m[m[0]++] = RUN;
                        break;
                case READ:
                        m[1]--;
                        for (w =
                             fscanf(tobj->input, "%s", s) < 1 ? exit(0),
                             0 : tobj->L; strcmp(s, &s[m[w + 1]]); w = m[w]) ;
                        if (w - 1) {
                                *x = w + 2;
                                if (m[8] == 0 && m[*x] == COMPILE) {
                                        (*x)++;
                                }
                                goto INNER;
                        } else {
                                if (m[8] != 0) {
                                        m[m[0]++] = 2;
                                        m[m[0]++] = atoi(s);
                                } else {
                                        *++S = f;
                                        f = atoi(s);
                                }
                        }
                        break;
                case LOAD:
                        f = m[f];
                        break;
                case STORE:
                        m[f] = *S--;
                        f = *S--;
                        break;
                case SUBTRACT:
                        f = *S-- - f;
                        break;
                case ADD:
                        f = *S-- + f;
                        break;
                case MULTIPLY:
                        f *= *S--;
                        break;
                case DIVIDE:
                        f = *S-- / f;
                        break;
                case LESSZ:
                        f = 0 > f;
                        break;
                case EXIT:
                        I = m[m[1]--];
                        break;
                case EMIT:
                        fputc(f, tobj->output);
                        f = *S--;
                        break;
                case KEY:
                        *++S = f;
                        f = fgetc(tobj->input);
                        break;
                case FROMR:
                        *++S = f;
                        f = m[m[1]--];
                        break;
                case TOR:
                        m[++m[1]] = f;
                        f = *S--;
                        break;
                case BRANCH:
                        I += m[I];
                        break;
                case NBRANCH:
                        if (f == 0) {
                                I += m[I];
                        } else {
                                I++;
                        }
                        f = *S--;
                        break;
                case PRINTNUM:
                        fprintf(tobj->output, "%d", f);
                        f = *S--;
                        break;
                case QUOTE:
                        *++S = f;
                        f = m[I++];
                        break;
                case COMMA:
                        m[m[0]++] = f;
                        f = *S--;
                        break;
                case NOT:
                        f = !f;
                        break;
                case EQUAL:
                        f = *S-- == f;
                        break;
                case SWAP:
                        w = f;
                        f = *S--;
                        *++S = w;
                        break;
                case DUP:
                        *++S = f;
                        break;
                case DROP:
                        f = *S--;
                        break;
                case TAIL:
                        m[1]--;
                        break;
                default:
                        fprintf(stderr, "Unknown instruction\n");
                        exit(1);
                }
        }
}
