/* 
 * https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Longest_common_subsequence
 * http://www.algorithmist.com/index.php/Longest_Common_Subsequence
 * https://en.wikipedia.org/wiki/Longest_common_subsequence_problem
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

typedef struct {
        char *c;
        size_t m, n;
} diff;

diff *lcs(char *x, char *y)
{
        diff *d;
        size_t m, n, i, j;
        char *c;
        assert(x && y);
        m = strlen(x);
        n = strlen(y);
        if(!(c = calloc((m+1)*(n+1),1)))
                return perror("calloc failed"), NULL;
        if(!(d = calloc(1, sizeof(*d))))
                return perror("calloc failed"), NULL;
        for(i = 1; i < m + 1; i++)
                for(j = 1; j < n + 1; j++)
                        if (x[i-1] == y[j-1])
                                c[(i*m)+j] = c[((i-1)*m)+(j-1)] + 1;
                        else
                                c[(i*m)+j] = MAX(c[(i*m)+(j-1)],c[((i-1)*m)+j]);
        d->c = c;
        d->m = m;
        d->n = n;
        return d;
}

void print_diff_inner(diff *d, char *x, char *y, size_t i, size_t j)
{
        if (i > 0 && j > 0 && x[i-1] == y[j-1]) {
                print_diff_inner(d, x, y, i-1, j-1);
                printf("  %c\n", x[i-1]);
        } else {
                if (j > 0 && (i == 0 || d->c[(i*(d->m))+(j-1)] >= d->c[((i-1)*(d->m))+j])) {
                        print_diff_inner(d, x, y, i, j-1);
                        printf("+ %c\n", y[j-1]);
                } else if(i > 0 && (j == 0 || d->c[(i*(d->m))+(j-1)] < d->c[((i-1)*(d->m))+j])) {

                        print_diff_inner(d, x, y, i-1, j);
                        printf("- %c\n", x[i-1]);
                }
        }

}

void print_diff(diff *d, char *x, char *y)
{
        assert(d && x && y);
        print_diff_inner(d, x, y, d->m, d->n);
}

int main(int argc, char **argv) 
{
        diff *c;
        size_t m,n,i;
        if(argc != 3)
                return fprintf(stderr, "usage %s s1 s2\n", argv[0]), -1;
        printf("%s:%s\n",argv[1], argv[2]);
        c = lcs(argv[1], argv[2]);
        print_diff(c, argv[1], argv[2]);
        return 0;
}

