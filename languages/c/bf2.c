/* @author Richard Howe
 * brainfuck interpreter <https://en.wikipedia.org/wiki/Brainfuck>*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define BF_LEN (32768u)

int bf(char *s, int8_t *m, size_t len, FILE *in, FILE *out) {
        size_t pc = 0, p = 0;
        long long int bal = 0, c;
        int8_t i;
        while((i = s[pc++]))
                switch(i) {
                case '>': ++p; p %= len; break;
                case '<': --p; p %= len; break;
                case '+': ++m[p]; break;
                case '-': --m[p]; break;
                case '.': if(EOF == fputc(m[p], out)) return -1; break;
                case ',': if(EOF == (c = fgetc(in)))  return -1; m[p] = c; break;
                case '[': if(!m[p])
                                  for(bal = 1, pc++; (i = s[pc]); pc++) {
                                          if(i == '[') bal++;
                                          if(i == ']') bal--;
                                          if(!bal) { pc++; break; }
                                  }
                          if(!i) return fprintf(stderr, "unbalanced '['\n"), -2; 
                          break;
                case ']': if(m[p])
                                  for(bal = -1, pc-=2; (i = s[pc]) && pc < len; pc--){
                                          if(i == '[') bal++;
                                          if(i == ']') bal--;
                                          if(!bal) { pc++; break; }
                                  }
                          if(pc > len) return fprintf(stderr, "unbalanced ']'\n"), -2; 
                          break;
                default: break;
                }
        return pc;
}

int main(int argc, char **argv) {
        FILE *pfile;
        int8_t bmem[BF_LEN] = {0}; /*allocates a lot on the stack...*/
        char   pmem[BF_LEN] = {0};
        if(argc == 1)
                return fprintf(stderr, "usage: %s file.bf\n", argv[0]), -1;
        while(argv++, --argc) {
                memset(pmem, 0, BF_LEN);
                memset(bmem, 0, BF_LEN);
                if(!(pfile = fopen(argv[0], "rb")))
                        return perror(argv[0]), -1;
                (void)fread(pmem, 1, BF_LEN, pfile);
                fclose(pfile);
                pmem[BF_LEN-1] = '\0';
                bf(pmem, bmem, BF_LEN, stdin, stdout);
        }
        return 0;
}
