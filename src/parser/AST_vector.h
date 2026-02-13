#ifndef AST_VECTOR_H
#define AST_VECTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "../throwErr.h"
#include "../lexer/token.h"

typedef struct Ast_node Ast_node;
typedef struct AST_vector AST_vector;

/*
---
## head
type - `Ast_node`

#### members
parent - `NULL`
children - type `AST_vector`; holds member `data` (type `Ast_node`) of all of the children

---
## children
type - `Ast_node`

#### members
parent - pointer to the parent of that node
children - type `AST_vector`; holds member `data` (type `Ast_node`) of all of the children
---
*/

typedef struct Ast_node {
    Token *token;
    Ast_node *parent;
    AST_vector *children;
};

typedef struct AST_vector {
    Ast_node *data; // pointer array
    unsigned int size;
    unsigned int capacity;
};

#endif // AST_VECTOR_H