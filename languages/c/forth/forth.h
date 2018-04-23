struct third_obj {
        FILE *input, *output;
        char s[2500];
        int m[2500], L, I, T[500], *S, t, w, f, x;
};

typedef struct third_obj third_obj_t;

enum primitives {
        PUSH, COMPILE, RUN, DEFINE, IMMEDIATE, READ, LOAD, STORE, SUBTRACT, ADD,
        MULTIPLY, DIVIDE, LESSZ, EXIT, EMIT, KEY, FROMR, TOR, BRANCH, NBRANCH,
        PRINTNUM, QUOTE, COMMA, NOT, EQUAL, SWAP, DUP, DROP, TAIL, LAST_ENUM
};

enum bool { false, true };

/*external facing function prototypes*/
int init_third(third_obj_t * tobj, FILE * input, FILE * output);
void run_third(third_obj_t * tobj);
