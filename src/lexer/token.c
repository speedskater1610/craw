#include "token.h"

// Constructor
Token *Token_new(enum TokenType tokenType, char* lexeme, unsigned int line, unsigned int column) {
    Token *s = (Token*)malloc(sizeof(Token));
    if (s != NULL) {
        s->tokenType = tokenType;
        s->lexeme = lexeme;
        s->line = line;
        s->column = column;
    }
    return s;
}

enum TokenType from_keyword(const char *s) {
    if (!strcmp(s, "let")) return Let;
    else if (!strcmp(s, "fn")) return Fn;
    else if (!strcmp(s, "return")) return Return;
    else if (!strcmp(s, "if")) return If;
    else if (!strcmp(s, "goto")) return Goto;
    else if (!strcmp(s, "lbl")) return Lbl;
    else if (!strcmp(s, "new")) return New;
    else if (!strcmp(s, "this")) return This;
    else if (!strcmp(s, "asm")) return Asm;
    else if (!strcmp(s, "use")) return Use;
    else if (!strcmp(s, "i16")) return I16;
    else if (!strcmp(s, "i32")) return I32;
    else if (!strcmp(s, "i64")) return I64;
    else if (!strcmp(s, "u16")) return U16;
    else if (!strcmp(s, "u32")) return U32;
    else if (!strcmp(s, "u64")) return U64;
    else if (!strcmp(s, "f32")) return F32;
    else if (!strcmp(s, "f64")) return F64;
    else if (!strcmp(s, "char")) return Char;
    else if (!strcmp(s, "structinstance")) return StructInstance;
    else if (!strcmp(s, "defstruct")) return DefStruct;
    else if (!strcmp(s, "void")) return Void;
    else if (!strcmp(s, "and")) return And;
    else if (!strcmp(s, "or")) return Or;
    else return Error;
}

// get the value functions
enum TokenType get_tokenType(const Token *s) {
    return s->tokenType; 
}

char *get_lexeme(const Token *s) {
    return s->lexeme; 
}

unsigned int get_line(const Token *s) {
    return s->line; 
}

unsigned int get_column(const Token *s) {
    return s->column; 
}

// set the value functions
void set_tokenType(Token *s, enum TokenType new_value) {
    s->tokenType = new_value;
}

void set_lexeme(Token *s, char *new_value) {
    s->lexeme = new_value;
}

void Token_set_line(Token *s, unsigned int new_value) {
    s->line = new_value;
}

void Token_set_column(Token *s, unsigned int new_value) {
    s->column = new_value;
}


// Destructor
void Token_free(Token *s) {
    free(s);
}
