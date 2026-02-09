#ifndef VECTOR_H
#define VECTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "AST_node.h"
#include "../throwErr.h"
#include "../lexer/token.h"

typedef struct {
    Ast_node* data;            // Pointer to the array of data
    unsigned int size;      // Current number of elements in the vector
    unsigned int capacity;  // Total capacity of the vector
} AST_vector;

// create a new vector
AST_vector *create_vector();
//  add an element to the AST_vector(push_back)
void push(AST_vector *vector, Ast_node value);
// rm last element
void pop(AST_vector *vector);
// fn to access an element at a specific index (vector_at)
Token vector_at(AST_vector *vector, unsigned int index);
// free the tokens when they are no longer needed
void vector_free(AST_vector *vector);

#endif // VECTOR_H