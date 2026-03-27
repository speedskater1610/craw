#ifndef AST_TOKEN
#define AST_TOKEN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nodes/function.h"
#include "nodes/program.h"
#include "nodes/var_dec.h"

// Simulated Vec<AstNode>
typedef struct AstNode AstNode;
typedef struct AstVec AstVec;

typedef struct {
    AstNode *nodes;
    size_t length;
    size_t capacity;
} AstVec;

// discriminant (tag)
typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION,
} AstNodeType;

// main enum struct
struct AstNode {
    AstNodeType type;
    union {
        ProgramData program;
        FunctionData function;
    } data;
};

typedef struct {} AssignmentData;
typedef struct {} BinaryOpData;
typedef struct {}UnaryOpData;
typedef struct {} FunctionCallData;
typedef struct {} ReturnData;
typedef struct {} IfData;
typedef struct {} LabelData;
typedef struct {} GotoData;
typedef struct {} IdentifierData;
typedef struct {} IntLiteralData;
typedef struct {} FloatLiteralData;
typedef struct {} CharLiteralData;
typedef struct {} StringLiteralData;
typedef struct {} ArrayLiteralData;
typedef struct {} MemberAccessData;
typedef struct {} ArrayAccessData;
typedef struct {} StructDefData;
typedef struct {} ClassDefData;
typedef struct {} NewData;
typedef struct {} AsmBlockData;
typedef struct {} ScopeData;
typedef struct {} TypeCastData;

typedef enum AST_token {
    Program,
    Function,
    VariableDecl,
    Assignment,
    BinaryOp,
    UnaryOp,
    FunctionCall,
    Return,
    If,
    Label,
    Goto,
    Identifier,
    IntLiteral,
    FloatLiteral,
    CharLiteral,
    StringLiteral,
    ArrayLiteral,
    MemberAccess,
    ArrayAccess,
    StructDef,
    ClassDef,
    New,
    AsmBlock,
    Scope,
    TypeCast,
};

#endif // AST_TOKEN