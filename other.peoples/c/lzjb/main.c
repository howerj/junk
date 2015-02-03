#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "compress.h"

#define GENERIC_BUF_LEN (4096u)

static char buf[GENERIC_BUF_LEN]; 
static char compressed[GENERIC_BUF_LEN];
static size_t clen = 0, ctotal = 0, ilen = 0, total = 0;

void fail_if(int test, char *msg){
       if(test){
              if(errno && msg)
                      perror(msg);
              exit(-1);
       }
}

size_t fill_buf(FILE *input){ 
        size_t r;
        if(!ferror(input) && !feof(input) && (r = fread(buf, 1, GENERIC_BUF_LEN, input)))
                return r;
        return 0;
}

int main(int argc, char **argv){
        FILE *in, *out;
        if(argc > 1){
                while(++argv, --argc){
                        fail_if(NULL == (in  = fopen(argv[0], "rb")), argv[0]);
                        fail_if(NULL == (out = fopen("out.X", "wb")), "out.X");

                        while(GENERIC_BUF_LEN == (ilen = fill_buf(in))){
                                clen    = compress(buf, compressed, GENERIC_BUF_LEN);
                                fwrite(compressed, 1, clen, out);
                                total  += ilen;
                                ctotal += clen;
                        }
                        clen    = compress(buf, compressed, ilen);
                        fwrite(compressed, 1, clen, out);
                        total  += ilen;
                        ctotal += clen;
                        
                        fclose(in);
                        fclose(out);
                        printf("ctotal/total = %u/%u = %f\n", ctotal, total, ctotal/(float)total);
                }
        } else {
                fprintf(stderr, "usage: %s file+\n", argv[0]);
                return -1;
        }
        return 0;
}
