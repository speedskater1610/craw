#include "interpreter/parser.h"
#include "interpreter/interpreter.h"
#include "../../throwErr.h"

#include "lisp.h"

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

char *eval_lisp(char *lisp_src) {
    LispInterpreter interpreter = NULL;
    bool success = interpreter_init(interpreter);

    if (!success) {
        Err(
            "\e[31m\e[1m\e[4m\e[40mLISP COMPTIME ERROR - \e[0m"
            " \e[4m\e[36m\e[40minvalid comptime lisp,\e[0m"
            " \e[37m\e[40m No interpretation could be confirmed .\e[0m"
            "\n\t\e[0m"
        );
        return NULL;
    }

    const_expression const_lisp_src = lisp_src;
    char *expression_value = interpret_expression(interpreter, const_lisp_src);

    return expression_value;
}