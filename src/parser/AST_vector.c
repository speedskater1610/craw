#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "../throwErr.h"
#include "../lexer/token.h"
#include "AST_vector.h"

void AST_vector_free(AST_vector *vector) {
    if (!vector) return; // NULL case

    if (vector->data) {
        for (size_t i = 0; i < vector->size; i++) {
            free(vector->data[i].token);
            free(vector->data[i].parent);
            free(vector->data[i].children);
        }

        free(vector->data);
    }

    vector->data = NULL;
    vector->size = 0;
    vector->capacity = 0;

    free(vector);
}

// create a new vector
AST_vector *AST_create_vector() {
    AST_vector *vector = (AST_vector*)malloc(sizeof(AST_vector));
    
    vector->data = NULL;
    vector->size = 0;
    vector->capacity = 0;
    
    return vector;
}


//  add an element to the vector (push_back)
void AST_push(AST_vector *vector, Ast_node value) {
    if (vector->data == NULL) {
        // if the vector is empty -> allocate memory for one element
        vector->data = (Ast_node*)malloc(sizeof(Ast_node) * 1);
        vector->capacity = 1;
    } else if (vector->size >= vector->capacity) {
        // if the vector is full -> double its capacity by reallocating memory
        vector->capacity *= 2;
        vector->data = (Ast_node*)realloc(vector->data, vector->capacity * sizeof(Ast_node));
    }
    
    // add the new element to the end of the vector
    vector->data[vector->size] = value;
    // inc size bc of new element
    vector->size++;
}

// rm last element
void AST_pop(AST_vector *vector) {
    if (vector->size > 0) vector->size--;
}