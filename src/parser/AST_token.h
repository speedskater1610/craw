#ifndef AST_TOKEN
#define AST_TOKEN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nodes/function.h"
#include "nodes/program.h"

// Simulated Vec<AstNode>
typedef struct AstNode AstNode;
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

typedef enum ProgramData {};
typedef enum VariableDeclData {};
typedef enum AssignmentData {};
typedef enum BinaryOpData {};
typedef enum UnaryOpData {};
typedef enum FunctionCallData {};
typedef enum ReturnData {};
typedef enum IfData {};
typedef enum LabelData {};
typedef enum GotoData {};
typedef enum IdentifierData {};
typedef enum IntLiteralData {};
typedef enum FloatLiteralData {};
typedef enum CharLiteralData {};
typedef enum StringLiteralData {};
typedef enum ArrayLiteralData {};
typedef enum MemberAccessData {};
typedef enum ArrayAccessData {};
typedef enum StructDefData {};
typedef enum ClassDefData {};
typedef enum NewData {};
typedef enum AsmBlockData {};
typedef enum ScopeData {};
typedef enum TypeCastData {};

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