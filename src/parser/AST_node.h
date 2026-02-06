#ifndef AST_NODE_H
#define AST_NODE_H

#include "AST_vector.h"

typedef struct {
    Token *token;
    Ast_node *parent;
    AST_vector *children;
} Ast_node;

#endif AST_NODE_H