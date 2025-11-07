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

enum TokenType from_keyword (char *s) {
    enum TokenType rtrn;
    if (!strcmp(s, "let")) {
        rtrn = Let; 
    } else if (!strcmp(s, "fn")) {
        rtrn = Fn; 
    } else if (!strcmp(s, "return")) {
        rtrn = Return; 
    } else if (!strcmp(s, "if")) {
        rtrn = If;
    } else if (!strcmp(s, "goto")) {
        rtrn = Goto; 
    } else if (!strcmp(s, "lbl")) {
        rtrn = Lbl; 
    } else if (!strcmp(s, "new")) {
        rtrn = New; 
    } else if (!strcmp(s, "this")) {
        rtrn = This; 
    } else if (!strcmp(s, "asm")) {
        rtrn = Asm; 
    } else if (!strcmp(s, "use")) {
        rtrn = Use; 
    } else if (!strcmp(s, "i16")) {
        rtrn = I16; 
    } else if (!strcmp(s, "i32")) {
        rtrn = I32; 
    } else if (!strcmp(s, "i64")) {
        rtrn = I64; 
    } else if (!strcmp(s, "u16")) {
        rtrn = U16; 
    } else if (!strcmp(s, "u32")) {
        rtrn = U32; 
    } else if (!strcmp(s, "u64")) {
        rtrn = U64;
    } else if (!strcmp(s, "f32")) {
        rtrn = F32; 
    } else if (!strcmp(s, "f64")) {
        rtrn = F64; 
    } else if (!strcmp(s, "char")) {
        rtrn = Char; 
    } else if (!strcmp(s, "auto")) {
        rtrn = Auto; 
    } else if (!strcmp(s, "structinstance")) {
        rtrn = StructInstance; 
    } else if (!strcmp(s, "defstruct")) {
        rtrn = DefStruct; 
    } else if (!strcmp(s, "obj")) {
        rtrn = Obj; 
    } else if (!strcmp(s, "class")) {
        rtrn = Class; 
    } else if (!strcmp(s, "void")) {
        rtrn = Void; 
    } else if (!strcmp(s, "and")) {
        rtrn = And; 
    } else if (!strcmp(s, "or")) {
        rtrn = Or; 
    } else {
        rtrn = Error; 
    }
    
    return rtrn;
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
