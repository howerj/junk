/* Example of using libtcc to compile and execute C code at run time.
 * Originally from <https://blog.mister-muffin.de/2011/10/22/discovering-tcc/>
 * however the API has changed since that post was made */
#include <stdlib.h>
#include <stdio.h>
#include "libtcc.h"

/*our example program wants an external function 'add'*/
int add(int a, int b) { return a + b; }

char my_program[] =
"extern int add(int a, int b); \n"
"int square(int n) {\n"
"    return n * n;\n"
"}\n"
"int foobar(int n) {\n"
"    printf(\"square(%d) = %d\\n\", n, square(n));\n"
"    printf(\"add(%d, %d) = %d\\n\", n, 2 * n, add(n, 2 * n));\n"
"    return 1337;\n"
"}\n";

int main(int argc, char **argv)
{
    TCCState *s;
    int (*foobar_func)(int);

    if(NULL == (s = tcc_new())){
            fprintf(stderr, "Allocation of a new TCC compilation enviroment failed\n");
            return 1;
    }

    tcc_set_output_type(s, TCC_OUTPUT_MEMORY); /*Do everything in memory!*/

    if(tcc_compile_string(s, my_program) > 0) 
            return -1;

    tcc_add_symbol(s, "add", add); /*add in that missing symbol*/

    tcc_relocate(s);

    foobar_func = tcc_get_symbol(s, "foobar");

    printf("foobar returned: %d\n", foobar_func(7));

    tcc_delete(s);

    return 0;
}
