#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "../lexer/vector.h"
#include "../lexer/token.h"
#include "AST.h"

/*
 * Parser state
 *
 *   tokens  - flat token vector produced by the lexer
 *   pos     - current read position into tokens
 *   had_error - set to true if any parse error was emitted
 */
typedef struct {
    Vector  *tokens;
    unsigned int pos;
    bool     had_error;
} Parser;

/* Lifecycle */
Parser   *parser_new(Vector *tokens);
void      parser_free(Parser *p);

/* Entry point */
Ast_node *parse(Parser *p);

#endif /* PARSER_H */