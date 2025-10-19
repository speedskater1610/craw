#include "token.h"

// Constructor
Token* Token_new(enum TokenType tokenType, char* lexeme, unsigned int line, unsigned int column) {
    Token* s = (Token*)malloc(sizeof(Token));
    if (s != NULL) {
        s->tokenType = tokenType;
        s->lexeme = lexeme;
        s->line = line;
        s->column = column;
    }
    return s;
}

// get the value functions
enum TokenType get_tokenType(const Token* s) {
    return s->tokenType; 
}

char* get_lexeme(const Token* s) {
    return s->lexeme; 
}

unsigned int get_line(const Token* s) {
    return s->line; 
}

unsigned int get_column(const Token* s) {
    return s->column; 
}

// set the value functions
void set_tokenType(Token* s, enum TokenType new_value) {
    s->tokenType = new_value;
}

void set_lexeme(Token* s, char* new_value) {
    s->lexeme = new_value;
}

void Token_set_line(Token* s, unsigned int new_value) {
    s->line = new_value;
}

void Token_set_column(Token* s, unsigned int new_value) {
    s->column = new_value;
}


// Destructor
void Token_free(Token* s) {
    free(s);
}
