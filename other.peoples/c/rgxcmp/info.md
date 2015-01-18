# <https://codegolf.stackexchange.com/questions/7024/compile-regexes?lq=1>
# C, 627 characters

This program treats its first command-line argument as the input and generates C code as its output.

        #define A(v) F[v]+strlen(F[v])
        #define S sprintf
        char*B="&&f%d(s)||f%d(s)",*J="&&f%d(s+%d)",*r,F[256][65536];h=2;e(f,n,o,R,C,O,t,g){for(C=O=f;*r;++r)switch(*r){case 40:r++;e(g=h++,C=h++,0,0);r[1]^42?t=g:(t=C,S(A(t),B,g,C=h++),r++);o=!S(A(O),J,t,o);O=C;break;case 41:goto L;case'|':S(A(C),J,n,o);o=!S(A(O=f),"||1");break;default:r[1]^42?S(A(C),"&&s[%d]==%d",o++,*r,O^f||R++):(o=!S(A(O),J,t=h++,o),O=C=h++,g=h++,S(A(g),"&&*s==%d&&f%d(s+1)",*r++,t),S(A(t),B,g,C));}L:S(A(C),J,n,o);}main(int c,char**v){r=v[1];for(e(1,!S(*F,"&&!*s"),0,0);h--;)printf("f%d(char*s){return 1%s;}",h,F[h]);puts("main(int c,char**v){exit(f1(v[1]));}");}

Here is its output for: 

        (0|1(0|1)*)(|A(0|1)*1) 

(with newlines added):

        f11(char*s){return 1&&s[0]==49&&f7(s+1);}
        f10(char*s){return 1&&s[0]==48&&f9(s+1)||1&&s[0]==49&&f9(s+1);}
        f9(char*s){return 1&&f10(s)||f11(s);}
        f8(char*s){return 1&&f7(s+0)||1&&s[0]==65&&f9(s+1);}
        f7(char*s){return 1&&f0(s+0);}
        f6(char*s){return 1&&f2(s+0);}
        f5(char*s){return 1&&s[0]==48&&f4(s+1)||1&&s[0]==49&&f4(s+1);}
        f4(char*s){return 1&&f5(s)||f6(s);}
        f3(char*s){return 1&&s[0]==48&&f2(s+1)||1&&s[0]==49&&f4(s+1);}
        f2(char*s){return 1&&f8(s+0);}
        f1(char*s){return 1&&f3(s+0);}
        f0(char*s){return 1&&!*s;}
        main(int c,char**v){exit(f1(v[1]));}

If you provide valid input as its first command-line argument, it returns exit status 1. Otherwise, it returns exit status 0.

    $ ./regexcompiler '(0|1(0|1)*)(|A(0|1)*1)' > floatprog.c
    $ gcc -o floatprog floatprog.c
    floatprog.c: In function ‘main’:
    floatprog.c:1:519: warning: incompatible implicit declaration of built-in function ‘exit’ [enabled by default]
    $ ./floatprog '1A00001' && echo invalid || echo valid
    valid
    $ ./floatprog '100A010' && echo invalid || echo valid
    invalid

Either program, if you fail to provide the command-line argument, dereferences a null pointer, causing a segmentation fault. A sufficiently long regex will overflow the buffers of this submission, and the size of input to a generated program is limited by the size of the stack. However, all the test cases work.

Note that e(g=h++,C=h++,0,0); introduces undefined behavior. If, for example, generated programs do not compile, you can try replacing the statement with h+=2;e(g=h-1,C=h-2,0,0);, which is five characters longer
