#include "vector.h"

// create a new vector
Vector* create_vector() {
    // Allocate memory for the Vector structure
    Vector* vector = (Vector*)malloc(sizeof(Vector));
    
    vector->data = NULL;   // set data pointer to NULL -> indicating empty vector
    vector->size = 0;     // init the size to 0 (no elements in the vector)
    vector->capacity = 0; // init the capacity to 0 (no memory allocated)
    return vector; // return the newly created vector
}


//  add an element to the vector (push_back)
void push(Vector* vector, Token value) {
    if (vector->data == NULL) {
        // if the vector is empty -> allocate memory for one element
        vector->data = (Token*)malloc(sizeof(Token) * 1);
        vector->capacity = 1;
    } else if (vector->size >= vector->capacity) {
        // if the vector is full -> double its capacity by reallocating memory
        vector->capacity *= 2;
        vector->data = (Token*)realloc(vector->data, vector->capacity * sizeof(Token));
    }
    
    // add the new element to the end of the vector
    vector->data[vector->size] = value;
    // inc size bc of new element
    vector->size++;
}

// rm last element
void pop(Vector* vector) {
    // unless the vector is empty  
    // decrement the size to remove the last element
    if (vector->size > 0) vector->size--;
}

// fn to access an element at a specific index (vector_at)
Token vector_at(Vector* vector, unsigned int index) {
    if (index >= vector->size) {
        // Check if the provided index is out of bounds
        Err (
    "\e[31m\e[1m\e[4m\e[40mCOMPILER ERROR - \e[0m" 
    " \e[4m\e[36m\e[40mInternal (vector_at function)\e[0m" 
    " \e[37m\e[40m Index out of bounds\e[0m" 
    "\n\t\e[0m"
    "\e[32m\e[64m\e[4mNOTE:\e[0m\e[32m\e[64mThis may or may not be because of a error in your code or in the compiler if this error persists then please open up a issue on my \e[0m"
    "\e[1m\e[4m\e[34m\e[47m\e]8;;https://github.com/speedskater1610/craw\e\\git-hub\e]8;;\e\\\e[0m\n"
        );
        exit(1);
    }
    return vector->data[index]; // return the element at the specified index
}
