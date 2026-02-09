#ifndef VECTOR_H
#define VECTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "token.h"
#include "../throwErr.h"

typedef struct {
    Token* data;            // Pointer to the array of data
    unsigned int size;      // Current number of elements in the vector
    unsigned int capacity;  // Total capacity of the vector
} Vector;

// create a new vector
Vector *create_vector();
//  add an element to the vector (push_back)
void push(Vector *vector, Token value);
// rm last element
void pop(Vector *vector);
// fn to access an element at a specific index (vector_at)
Token vector_at(Vector *vector, unsigned int index);
// free the tokens when they are no longer needed
void vector_free(Vector *vector);

#endif
