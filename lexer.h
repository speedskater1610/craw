#ifndef LEXER_H
#define LEXER_H

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "token.h"
#include "vector.h"
#include "result.h"

typedef struct {
    char* input;
    unsigned int position;
    unsigned int line;
    unsigned int column;
} Lexer;


// constructor
Lexer* Lexer_new (char* input);

#endif // LEXER_H
