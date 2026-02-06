#include "AST_vector.h"

void vector_free(AST_vector*vector) {
    if (!vector) return;

    if (vector->data) {
        // Free each Token's lexeme if it is dynamically allocated
        for (size_t i = 0; i < vector->size; i++) {
            free(vector->data[i].lexeme);
        }

        // Free the data array
        free(vector->data);
    }

    // Reset the AST_vectorfields
    vector->data = NULL;
    vector->size = 0;
    vector->capacity = 0;

    // Finally, free the AST_vectorstructure itself
    free(vector);
}

// create a new vector
AST_vector*create_vector() {
    // Allocate memory for the AST_vectorstructure
    AST_vector*AST_vector= (Vector*)malloc(sizeof(Vector));
    
    vector->data = NULL;   // set data pointer to NULL -> indicating empty vector
    vector->size = 0;     // init the size to 0 (no elements in the vector)
    vector->capacity = 0; // init the capacity to 0 (no memory allocated)
    return vector; // return the newly created vector
}


//  add an element to the AST_vector(push_back)
void push(AST_vector*vector, Ast_node value) {
    if (vector->data == NULL) {
        // if the AST_vectoris empty -> allocate memory for one element
        vector->data = (Token*)malloc(sizeof(Token) * 1);
        vector->capacity = 1;
    } else if (vector->size >= vector->capacity) {
        // if the AST_vectoris full -> double its capacity by reallocating memory
        vector->capacity *= 2;
        vector->data = (Token*)realloc(vector->data, vector->capacity * sizeof(Token));
    }
    
    // add the new element to the end of the vector
    vector->data[vector->size] = value;
    // inc size bc of new element
    vector->size++;
}

// rm last element
void pop(AST_vector*vector) {
    // unless the AST_vectoris empty  
    // decrement the size to remove the last element
    if (vector->size > 0) vector->size--;
}

// fn to access an element at a specific index (vector_at)
Token vector_at(AST_vector*vector, unsigned int index) {
    if (index >= vector->size) {
        // Check if the provided index is out of bounds
        fprintf(
        stderr,
    "\e[31m\e[1m\e[4m\e[40mCOMPILER ERROR - \e[0m" 
    " \e[4m\e[36m\e[40mInternal (vector_at function)\e[0m" 
    " \e[37m\e[40m Index out of bounds\e[0m" 
        );
        exit(1);
    }
    return vector->data[index]; // return the element at the specified index
}
