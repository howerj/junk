/* 
 * https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Longest_common_subsequence
 * http://www.algorithmist.com/index.php/Longest_Common_Subsequence
 * https://en.wikipedia.org/wiki/Longest_common_subsequence_problem
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define MAX(X,Y) (((X) > (Y)) ? (X) : (Y))

char *x[] = {
    "This part of the document has stayed",
    "the same from version to version.",
    "",
    "This paragraph contains text that is",
    "outdated - it will be deprecated '''and'''",
    "deleted '''in''' the near future.",
    "",
    "It is important to spell check this",
    "dokument. On the other hand, a misspelled",
    "word isn't the end of the world.",
    NULL
};

char *y[] = {
    "This is an important notice! It should",
    "therefore be located at the beginning of",
    "this document!",
    "",
    "This part of the document has stayed",
    "the same from version to version.",
    "",
    "It is important to spell check this",
    "document. On the other hand, a misspelled",
    "word isn't the end of the world. This",
    "paragraph contains important new",
    "additions to this document.",
    NULL
};

typedef struct {
        char *c;
        size_t m, n;
} diff;

diff *lcs(char *x[], char *y[])
{
        diff *d;
        char *c;
        size_t m=0, n=0, i, j;
        assert(x && y);
        while(x[++m]);
        while(y[++n]);
        if(!(c = calloc((m+1)*(n+1),1)))
                return perror("calloc failed"), NULL;
        if(!(d = calloc(1, sizeof(*d))))
                return perror("calloc failed"), NULL;
        for(i = 1; i <= m; i++)
                for(j = 1; j < n; j++) /*XXX This should be "<="? What did I do?*/
                        if (!strcmp(x[i-1], y[j-1]))
                                c[i*n+j] = c[(i-1)*n+(j-1)] + 1;
                        else
                                c[i*n+j] = MAX(c[i*n+(j-1)], c[(i-1)*n+j]);
        d->c = c;
        d->m = m;
        d->n = n;
        return d;
}

void print_diff_inner(diff *d, char *x[], char *y[], size_t i, size_t j)
{
        if (i > 0 && j > 0 && !strcmp(x[i-1], y[j-1])) {
                print_diff_inner(d, x, y, i-1, j-1);
                printf("  %s\n", x[i-1]);
        } else {
                if (j > 0 && (i == 0 || d->c[(i*(d->n))+(j-1)] >= d->c[((i-1)*(d->n))+j])) {
                        print_diff_inner(d, x, y, i, j-1);
                        printf("+ %s\n", y[j-1]);
                } else if(i > 0 && (j == 0 || d->c[(i*(d->n))+(j-1)] < d->c[((i-1)*(d->n))+j])) {
                        print_diff_inner(d, x, y, i-1, j);
                        printf("- %s\n", x[i-1]);
                }
        }
}

void print_diff(diff *d, char **x, char **y)
{
        assert(d && x && y);
        print_diff_inner(d, x, y, d->m, d->n);
}

int main(void) 
{
        size_t i,j;
        diff *d;
        d = lcs(x, y);
        print_diff(d, x, y);
        for(i = 0; i <= (d->m); i++) {
                putchar('\n');
                for(j = 0; j < d->n; j++)
                        printf("%d ", d->c[i*d->n+j]);
        }
        putchar('\n');
        return 0;
}

