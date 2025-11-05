#ifndef LEXER_H
#define LEXER_H

#define append_char(str, c) (str = append_char_impl(str, c))

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "token.h"
#include "vector.h"
#include "result.h"
#include "throwErr.h"

#include "f32.h"
#include "f64.h"

typedef struct {
    char* input;
    unsigned int position;
    unsigned int line;
    unsigned int column;
} Lexer;



int to_digit(char c, int radix);
char *append_char_impl(char *str, char c);
bool is_numeric(char s);
Lexer* Lexer_new (char* input);
bool is_at_end (Lexer* self);
void advance (Lexer* self);
char current_char(Lexer* self);
void skip_whitespace(Lexer* self);
ResultSigTok read_string_literal (Lexer* self, 
                            unsigned int start_line, 
                            unsigned int start_column);
ResultSigTok read_char_literal (Lexer* self, 
                            unsigned int start_line,
                            unsigned int start_column);
ResultSigTok read_number (Lexer* self, 
                    unsigned int start_line, 
                    unsigned int start_column);
ResultSigTok next_token(Lexer* self);
Vector tokenize(Lexer* self);

#endif // LEXER_H
