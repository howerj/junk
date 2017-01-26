/** @file       libprolog.h
 *  @brief      A prolog library, based on <www.ioccc.org/1992/buzzard.2.c>
 *  @author     Richard James Howe.
 *  @copyright  Copyright 2015 Richard James Howe.
 *  @license    LGPL v2.1 or later version
 *  @email      howe.r.j.89@gmail.com **/
#ifndef prolog_H
#define prolog_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
struct prolog_obj; /**An opaque object that holds a running prolog environment**/
typedef struct prolog_obj prolog_obj_t;
/** @brief   Given an input and an output this will initialize prolog_obj_t,
 *           allocating memory for it and setting it up so it has a few
 *           prolog words predefined. The returned object must be free'd
 *           by the caller and can be done with free(). It will return NULL
 *           on failure.
 *  @param   input       Read from this input file. Caller closes.
 *  @param   output      Output to this file. Caller closes.
 *  @return  prolog_obj_t A fully initialized prolog environment or NULL. **/
prolog_obj_t *prolog_init(FILE *input, FILE *output); 
/** @brief   Execute an initialized prolog environment, this will read
 *           from input until there is no more or an error occurs. If
 *           an error occurs a negative number will be returned and the
 *           prolog object passed to prolog_run will be invalidated, all
 *           subsequent calls to prolog_run() or prolog_eval() will return
 *           errors.
 *  @param   o   An initialized prolog environment. Caller frees.
 *  @return  int This is an error code, less than one is an error. **/
int prolog_run(prolog_obj_t *o); 
/** @brief   This function behaves like prolog_run() but instead will
 *           read from a string until there is no more. It will like-
 *           wise invalidate objects if there is an error evaluating the
 *           string. Do not forget to call either prolog_seti() or prolog_seto(),
 *           or to close any previous files passed to prolog_eval() after
 *           you have called prolog_eval(). Multiple calls to prolog_eval()
 *           will work however.
 *  @param   o   An initialized prolog environment. Caller frees.
 *  @param   s   A NUL terminated string to read from.
 *  @return  int This is an error code, less than one is an error. **/
int prolog_eval(prolog_obj_t *o, const char *s); 
/** @brief Set the input of an environment 'o' to read from a file 'in'.
 *  @param o   An initialized prolog environment. Caller frees.
 *  @param in  Open handle for reading; "r"/"rb". Caller closes. **/
void prolog_seti(prolog_obj_t *o, FILE *in);
/** @brief Set the output file of an environment 'o'.
 *  @param o   An initialized prolog environment. Caller frees. Asserted.
 *  @param out Open handle for writing; "w"/"wb". Caller closes. Asserted. **/
void prolog_seto(prolog_obj_t *o, FILE *out);    
/** @brief Set the input of an environment 'o' to read from a string 's'.
 *  @param o   An initialized prolog environment. Caller frees. Asserted.
 *  @param s   A NUL terminated string to act as input. Asserted. **/
void prolog_sets(prolog_obj_t *o, const char *s); 
/** @brief  This implements a prolog REPL whose behavior is documented in
 *          the man pages for this library. You pass in the same format as
 *          is expected to main().  
 *  @param  argc  An argument count, like in main().
 *  @param  argv  argc strings, like in main(). Not checked for NULL.
 *  @return int   A error code. Anything non zero is an error. **/
int main_prolog(int argc, char **argv); 
#ifdef __cplusplus
}
#endif
#endif
