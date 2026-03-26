#ifndef FUNCTION_H
#define FUNCTION_H

#include "../AST_token.h"

typedef struct {
    char *name;
    char *type;
} Param;

typedef struct {
    Param *params;
    size_t length;
} ParamVec;

typedef struct {
    char *name;
    char *return_type;
    ParamVec params;
    AstVec body;
} FunctionData;

#endif