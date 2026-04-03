#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "AST.h"


/* NodeList */


void nodelist_init(NodeList *nl) {
    nl->items    = NULL;
    nl->size     = 0;
    nl->capacity = 0;
}

void nodelist_push(NodeList *nl, Ast_node *node) {
    if (nl->size >= nl->capacity) {
        size_t new_cap = (nl->capacity == 0) ? 4 : nl->capacity * 2;
        nl->items = realloc(nl->items, new_cap * sizeof(Ast_node *));
        nl->capacity = new_cap;
    }
    nl->items[nl->size++] = node;
}

void nodelist_free(NodeList *nl) {
    free(nl->items);
    nl->items    = NULL;
    nl->size     = 0;
    nl->capacity = 0;
}


/* Ast_node */


Ast_node *ast_node_new(NodeKind kind, Token *token) {
    Ast_node *n = malloc(sizeof(Ast_node));
    n->kind   = kind;
    n->token  = token;
    n->parent = NULL;
    nodelist_init(&n->children);
    return n;
}

void ast_add_child(Ast_node *parent, Ast_node *child) {
    if (!parent || !child) return;
    child->parent = parent;
    nodelist_push(&parent->children, child);
}

void ast_free(Ast_node *node) {
    if (!node) return;
    for (size_t i = 0; i < node->children.size; i++)
        ast_free(node->children.items[i]);
    nodelist_free(&node->children);
    /* tokens are owned by the token vector; do not free here */
    free(node);
}


/* Debug printer */

static const char *kind_name(NodeKind k) {
    switch (k) {
        case NODE_PROGRAM:        return "PROGRAM";
        case NODE_FN_DEF:         return "FN_DEF";
        case NODE_LET:            return "LET";
        case NODE_RETURN:         return "RETURN";
        case NODE_IF:             return "IF";
        case NODE_WHILE:          return "WHILE";
        case NODE_GOTO:           return "GOTO";
        case NODE_LABEL:          return "LABEL";
        case NODE_USE:            return "USE";
        case NODE_ASM_BLOCK:      return "ASM_BLOCK";
        case NODE_EXPR_STMT:      return "EXPR_STMT";
        case NODE_DEF_STRUCT:     return "DEF_STRUCT";
        case NODE_STRUCT_FIELD:   return "STRUCT_FIELD";
        case NODE_STRUCT_NEW:     return "STRUCT_NEW";
        case NODE_BINARY_OP:      return "BINARY_OP";
        case NODE_UNARY_OP:       return "UNARY_OP";
        case NODE_CALL:           return "CALL";
        case NODE_INDEX:          return "INDEX";
        case NODE_FIELD_ACCESS:   return "FIELD_ACCESS";
        case NODE_ASSIGN:         return "ASSIGN";
        case NODE_IDENTIFIER:     return "IDENTIFIER";
        case NODE_INT_LITERAL:    return "INT_LITERAL";
        case NODE_FLOAT_LITERAL:  return "FLOAT_LITERAL";
        case NODE_FLOAT32_LITERAL:return "FLOAT32_LITERAL";
        case NODE_CHAR_LITERAL:   return "CHAR_LITERAL";
        case NODE_STRING_LITERAL: return "STRING_LITERAL";
        case NODE_ARRAY_LITERAL:  return "ARRAY_LITERAL";
        case NODE_TYPE:           return "TYPE";
        case NODE_PARAM:          return "PARAM";
        default:                  return "UNKNOWN";
    }
}

void ast_print(const Ast_node *node, int indent) {
    if (!node) return;
    for (int i = 0; i < indent; i++) printf("  ");
    printf("[%s]", kind_name(node->kind));
    if (node->token && node->token->lexeme && node->token->lexeme[0] != '\0')
        printf(" \"%s\"", node->token->lexeme);
    printf("\n");
    for (size_t i = 0; i < node->children.size; i++)
        ast_print(node->children.items[i], indent + 1);
}