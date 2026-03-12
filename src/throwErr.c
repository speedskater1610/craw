#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include "throwErr.h"

void Err(const char *restrict format, ...) {
    va_list args;
    va_start(args, format);

    // copy args because vsnprintf will consume it
    va_list args_copy;
    va_copy(args_copy, args);

    int needed = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    if (needed < 0) {
        perror("vsnprintf");
        va_end(args);
        return;
    }

    char *buffer = malloc((size_t)needed + 1);
    if (!buffer) {
        perror("malloc");
        va_end(args);
        return;
    }

    vsnprintf(buffer, needed + 1, format, args);

    perror(buffer);

    free(buffer);
    va_end(args);

    has_at_least_one_error = true;
    printf("\n");
}

/*
 * Never go back into the git history of this file please I am begging you.
 * */
