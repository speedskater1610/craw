#ifndef RESULT_H
#define RESULT_H

#include <stdbool.h>

#include "vector.h"
#include "token.h"

typedef struct {
    Vector vector;
    bool Error;
} ResultVec;

typedef struct {
    Token token;
    bool Error;
} ResultSigTok;

#endif