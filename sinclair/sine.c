#include <stdio.h>
#include <stdlib.h>
/*
 * See: http://home.pipeline.com/~hbaker1/hakmem/hacks.html#item149
 * And: http://files.righto.com/calculator/sinclair_scientific_simulator.html
 *
 */

typedef enum {
        false,
        true
} bool;

typedef struct {
        float c;
        float s;
} cs;

#define MYPI (3.14592f)
#define STEP (MYPI/50.0f)

#define C_RED_M     "\x1b[31;1m"
#define C_GREEN_M   "\x1b[32;1m"
#define C_BLUE_M    "\x1b[34;1m"
#define C_DEFAULT_M "\x1b[0m"

#define GRAPH_LEN_M (60u)

#ifdef __GNUC__
#include <unistd.h>
#else
int isatty(int f);
int isatty(int f)
{
        return true;
}
#endif

cs mysine(float angle)
{
#define DIVISOR_M (1024.0f) /*set to 16.0f to see some cross over*/
        cs res;
        float c = 1.0f, s = 0.0f;
        float current_angle = (1.0f / (2 * DIVISOR_M)); 
        while (current_angle < angle) {
                c = c - (s / DIVISOR_M);
                s = s + (c / DIVISOR_M);
                current_angle += (1.0f / DIVISOR_M);
        }
        res.c = c;
        res.s = s;
        return res;
#undef DIVISOR_M
}

void print_bar(float c, float s, int width, bool color, FILE * out)
{
#define PRNT_COLOR_M(X)     if(true==color){ fprintf(out,X); }
#define DEFAULT_COLOR_M()   if(true==color){ fprintf(out,C_DEFAULT_M); }
        int i, j, k, l, cflag, tchar = 0;

        /*decide offset from beginning of line to print a dot */
        j = (c * ((float)width / 2u)) + ((float)width / 2u) - 1u;
        k = (s * ((float)width / 2u)) + ((float)width / 2u) - 1u;

        /*print out the left most dot first, which ever one it is */
        for (i = 0; (i < ((j < k) ? j : k)) && (i < width); i++) {
                putc(' ', out);
        }
        tchar += i;

        /*Used to decide which dot to print out in the next loop */
        if ((k - j) > 0) {
                l = k - j;
                cflag = true;
        } else {
                l = j - k;
                cflag = false;
        }

        PRNT_COLOR_M(j != k ? (cflag ? C_RED_M : C_GREEN_M):C_BLUE_M);
        putc('*', out);
        DEFAULT_COLOR_M();

        for (i = 0; i < l && (i < width); i++) {
                putc(' ', out);
        }
        tchar += i;

        PRNT_COLOR_M(cflag ? C_GREEN_M : C_RED_M);
        if(j!=k)
            putc('*', out); 
        DEFAULT_COLOR_M();

        for (i = tchar + 2 /*2 comes form '*' printed twice */ ; i < width; i++) {
                putc(' ', out);
        }

        putc('=', out);

        PRNT_COLOR_M(C_GREEN_M);
        fprintf(out, "<%.3f>,", s);
        PRNT_COLOR_M(C_RED_M);
        fprintf(out, "<%.3f>", c);
        DEFAULT_COLOR_M();

        putc('\n', out);
#undef PRNT_COLOR
#undef DEFAULT_COLOR
}

int main(void)
{
        float i;
        cs res;

        for (i = 0.0f; i < 2u * MYPI; i += STEP) {
                res = mysine(i);
                print_bar(res.c, res.s, GRAPH_LEN_M, isatty(fileno(stdout)),
                          stdout);
        }

        fprintf(stdout,"\n");
        print_bar(-1.0f,-1.0f, GRAPH_LEN_M, isatty(fileno(stdout)),stdout);
        print_bar(-0.5f,-0.5f, GRAPH_LEN_M, isatty(fileno(stdout)),stdout);
        print_bar(0.0f,0.0f, GRAPH_LEN_M, isatty(fileno(stdout)),stdout);
        print_bar(0.5f,0.5f, GRAPH_LEN_M, isatty(fileno(stdout)),stdout);
        print_bar(1.0f,1.0f, GRAPH_LEN_M, isatty(fileno(stdout)),stdout);
        return EXIT_SUCCESS;
}
