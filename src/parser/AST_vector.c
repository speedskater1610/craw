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