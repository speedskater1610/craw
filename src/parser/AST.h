#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include <stddef.h>
#include "hashmap.h"
#include "../lexer/token.h"

/*
 * AST Node kinds - every syntactic construct the parser can produce.
 */
typedef enum {
    /* --- Top-level / statements --- */
    NODE_PROGRAM,            /* root: list of top-level declarations         */
    NODE_FN_DEF,             /* fn name -> ret_type (params) { body }        */
    NODE_LET,                /* let name : type = expr ;                     */
    NODE_RETURN,             /* return expr ;                                 */
    NODE_IF,                 /* if (cond) { then } [else { else }]          */
    NODE_WHILE,              /* while (cond) { body }                        */
    NODE_GOTO,               /* goto label ;                                  */
    NODE_LABEL,              /* lbl name :                                    */
    NODE_USE,                /* use "file" ;                                  */
    NODE_ASM_BLOCK,          /* asm { ... }                                   */
    NODE_EXPR_STMT,          /* bare expression: expr ;                       */
    NODE_COMPTIME_LISP_BLOCK,/* runtimelisp { ... } (eg. `runtimelisp { (+ 3 4) }`) */
    NODE_COMPTIME_EXPR_BLOCK,/* runtimeexpr { ... } (eg. `runtimeexpr { 3 + 4 }`) */

    /* --- Struct declarations --- */
    NODE_DEF_STRUCT,        /* let Name : defstruct = { fields } ;          */
    NODE_STRUCT_FIELD,      /* let name : type  (inside defstruct)          */
    NODE_STRUCT_NEW,        /* new StructName                                */

    /* --- Expressions --- */
    NODE_BINARY_OP,         /* left op right                                 */
    NODE_UNARY_OP,          /* op expr                                       */
    NODE_CALL,              /* callee(args)                                  */
    NODE_INDEX,             /* array[index]                                  */
    NODE_FIELD_ACCESS,      /* obj.field                                     */
    NODE_ASSIGN,            /* target = value                                */
    NODE_IDENTIFIER,        /* bare name                                     */
    NODE_INT_LITERAL,
    NODE_FLOAT_LITERAL,
    NODE_FLOAT32_LITERAL,
    NODE_CHAR_LITERAL,
    NODE_STRING_LITERAL,
    NODE_ARRAY_LITERAL,     /* { expr, expr, ... }                           */

    /* --- Type nodes --- */
    NODE_TYPE,              /* wraps a type token (I32, Array, Pointer, …)  */

    /* --- Function parameter --- */
    NODE_PARAM,             /* name : type                                   */

    NODE_UNKNOWN,
} NodeKind;

/* Forward declaration */
typedef struct Ast_node Ast_node;

/* Dynamic array of Ast_node pointers (children) */
typedef struct {
    Ast_node **items;
    size_t     size;
    size_t     capacity;
} NodeList;

/*
 * Core AST node.
 *   kind     - what construct it represents
 *   token    - primary token (NULL for synthetic nodes like NODE_PROGRAM)
 *   parent   - parent node (NULL for root)
 *   children - ordered child nodes
 */
struct Ast_node {
    NodeKind    kind;
    Token      *token;
    Ast_node   *parent;
    NodeList    children;
};

/* ---------- NodeList helpers ---------- */
void      nodelist_init (NodeList *nl);
void      nodelist_push (NodeList *nl, Ast_node *node);
void      nodelist_free (NodeList *nl);

/* ---------- Ast_node helpers ---------- */
Ast_node *ast_node_new  (NodeKind kind, Token *token);
void      ast_add_child (Ast_node *parent, Ast_node *child);
void      ast_free      (Ast_node *node);

/* ---------- Debug printer ---------- */
void      ast_print     (const Ast_node *node, int indent);

#endif /* AST_H */