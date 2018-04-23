#include <stdio.h>
int main() {
    unsigned char add[] = { 0x8D, 0x04, 0x37, 0xC3 };
    int s = ((int(*)(int, int))add)(5, 7);
    printf("%d\n", s);
}
