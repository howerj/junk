#include <stdlib.h>
#include <stdio.h>
#include "forth.h"

int main(void)
{
        third_obj_t *tobj = calloc(1, sizeof(struct third_obj));
        init_third(tobj, stdin, stdout);
        run_third(tobj);
        free(tobj);
        return 0;
}
