#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

enum TokenType {
    // Keywords
    Let,
    Fn,
    Return,
    If,
    Goto,
    Lbl,
    New,
    This,
    Asm,
    Use,

    // Types
    I16,
    I32,
    I64,
    U16,
    U32,
    U64,
    F32,
    F64,
    Char,
    StructInstance,
    DefStruct,
    Void,

    // Type modifiers
    Array,      // a<type>
    Pointer,    // p<type>

    // Operators
    Plus,           // +
    Minus,          // -
    Star,           // *
    Slash,          // /
    Percent,        // %
    BitwiseAnd,     // &
    BitwiseOr,      // |
    BitwiseXor,     // ^
    BitwiseNot,     // ~
    LeftShift,      // <<
    RightShift,     // >>

    // Comparison & Logical
    Equal,          // ==
    NotEqual,       // !=
    LessThan,       // <
    GreaterThan,    // >
    LessEqual,      // <=
    GreaterEqual,   // >=
    And,            // and
    Or,             // or
    Not,            // !

    // Delimiters
    LeftParen,      // (
    RightParen,     // )
    LeftBrace,      // {
    RightBrace,     // }
    LeftBracket,    // [
    RightBracket,   // ]
    Semicolon,      // ;
    Comma,          // ,
    Dot,            // .
    Colon,          // :
    Arrow,          // ->
    Assign,         // =
    Hash,           // #

    // Literals
    IntLiteral,     // i64
    FloatLiteral,   // f64
    Float32Literal, // f32
    CharLiteral,    // char
    StringLiteral,  // Array char  (string)

    // Identifiers
    Identifier,     // Array char (string)

    // Special
    Eof,
    Newline,
    
    Error,          // something when wrong and the progarm should stop paring
};

typedef struct {
    enum TokenType tokenType;
    char *lexeme;
    unsigned int line;
    unsigned int column;
} Token;

// Constructor
Token* Token_new(enum TokenType tokenType, char *lexeme, unsigned int line, unsigned int column);

// get the value functions
enum TokenType get_tokenType(const Token *s);
enum TokenType from_keyword (char *s);
char* get_lexeme(const Token *s);
unsigned int get_line(const Token *s);
unsigned int get_column(const Token *s);

// set the value functions
void set_tokenType(Token *s, enum TokenType new_value);
void set_lexeme(Token *s, char *new_value);
void Token_set_line(Token *s, unsigned int new_value);
void Token_set_column(Token *s, unsigned int new_value);

// Destructor
void Token_free(Token *s);

#endif // TOKEN_H
