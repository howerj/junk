#include <stdlib.h>
#define SIZE (1024)

int (**hnew())[2];
void hdel(int (**e)[2]);
int (**hget(int (**t)[2], int k))[2];
void hset(int (**t)[2], int k, int v);

int (**hnew())[2] {
    return calloc(sizeof(int**), SIZE);
}
void hdel(int (**e)[2]) {
        int i;
        for (i = 0; i < SIZE; i++) 
                free(e[i]); 
        free(e);
}
int (**hget(int (**t)[2], int k))[2] {
        int h;
        for (h = k & (SIZE - 1); **t && ***t != k; h = ((h + 1) & (SIZE - 1)), t += h);
        return t;
}
void hset(int (**t)[2], int k, int v) {
        int (**a)[2];
        for (a = hget(t, k); !*a && (*a=malloc(sizeof(**t))); (**a)[0]=k,(**a)[1]=v);
}

// TEST DRIVER
#include <stdio.h>
int main(void) {
        int (**table)[2] = hnew();
        int (**a)[2], (**b)[2], (**c)[2];

        hset(table, 10, 20);
        hset(table, 20, 30);
        hset(table, 30, 40);

        a = hget(table, 10);
        b = hget(table, 20);
        c = hget(table, 30);

        printf("%d:%d\n", (**a)[0], (**a)[1]);
        printf("%d:%d\n", (**b)[0], (**b)[1]);
        printf("%d:%d\n", (**c)[0], (**c)[1]);

        hdel(table);
        return 0;
}
