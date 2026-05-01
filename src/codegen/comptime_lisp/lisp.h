#ifndef LISP_H
#define LISP_H

#include "interpreter/interpreter.h" // for `interpret_expression`

/**
* @param [in] lisp_src takes in a character array of a single lisp expression
* @return Returns the output of lisp
*/
char *eval_lisp(char *lisp_src, bool);

#endif /* LISP_H */