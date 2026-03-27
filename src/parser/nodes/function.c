#include "function.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void freeVec(ParamVec *pv) {
    free(pv.params);
    free(pv);
}

ParamVec new(size_t num_params, ...) {
    ParamVec vec;
    vec.length = num_params;
    vec.params = malloc(num_params * sizeof(Param));

    if (vec.params == NULL) {
        vec.length = 0;
        return vec; // Handle memory allocation failure
    }

    va_list args;
    va_start(args, num_params);
    for (size_t i = 0; i < num_params; ++i) {
        // Retrieve next Param object from va_list
        vec.params[i] = va_arg(args, Param);
    }

    va_end(args);
    return vec;
}