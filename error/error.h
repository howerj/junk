#include "stdio.h"

#ifndef error_h
#define error_h

typedef enum{
    false,
    true
} bool;

typedef unsigned int uint;

extern char *error_string[];

/* X MACRO ==================================================================*/

#define ERROR_TABLE \
  X(error_ok,, "No error")\
  X(error_general,, "Unknown error occured")\
  X(error_abort,, "Error occured, aborting")\
  X(error_eof,, "EOF")\
  X(error_malloc,, "Allocation failed")\
  X(error_null,, "Expect non-NULL pointer")\
  X(error_array_index,, "Array index exceeded bounds")\
  X(error_illegal_op,, "An illegal operation occured")\
  X(error_default_case,, "'default' in a case statement was selected")\
  X(error_failed_assert,, "An assertion failed")\
  X(error_file_not_found,, "File name could not be found")\
  X(error_division_by_zero,, "Division by zero")\
  X(error_modulo_by_zero,, "Modulo by zero")\
  X(error_not_an_error,,"Invalid error number")\
  X(error_no_errorno,,"No error message for error")\
  X(error_error,,"Error reporting failed")\
  X(error_LAST_ERROR,,"This should be the last error number")

#define X(a, b, c) a,
typedef enum{
  ERROR_TABLE 
} errornumber_t;
#undef X

/* X MACRO ==================================================================*/

typedef struct{
  FILE          *stderr_output;
  const char    *file;
  uint line;
  uint level;
  errornumber_t error_enum;
  bool color;
  bool silent;
} error_t;

uint handle_error(error_t *e, uint line, char* file, errornumber_t en);

#endif
