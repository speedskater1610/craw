/*
 * the recursive-descent parser for CRAW.
 *
 * Grammar (informal):
 *
 *   program        ::= top_decl* EOF
 *
 *   top_decl       ::= fn_def
 *                    | let_stmt
 *                    | use_stmt
 *
 *   fn_def         ::= 'fn' IDENT '->' type '(' param_list ')' block
 *   param_list     ::= (param (',' param)*)? 
 *   param          ::= IDENT ':' type
 *
 *   block          ::= '{' stmt* '}'
 *
 *   stmt           ::= let_stmt
 *                    | return_stmt
 *                    | if_stmt
 *                    | goto_stmt
 *                    | label_stmt
 *                    | use_stmt
 *                    | asm_block
 *                    | expr_stmt
 *
 *   let_stmt       ::= 'let' IDENT ':' type ('=' expr)? ';'
 *                    | 'let' IDENT ':' 'defstruct' '=' '{' struct_field* '}' ';'
 *
 *   struct_field   ::= 'let' IDENT ':' type ','?
 *
 *   return_stmt    ::= 'return' expr? ';'
 *   if_stmt        ::= 'if' '(' expr ')' block
 *   goto_stmt      ::= 'goto' IDENT ';'
 *   label_stmt     ::= 'lbl' IDENT ':'
 *   use_stmt       ::= 'use' STRING_LITERAL ';'
 *   asm_block      ::= 'asm' '{' <raw tokens until matching '}'>  '}'
 *   expr_stmt      ::= expr ';'
 *
 *   type           ::= 'i16'|'i32'|'i64'|'u16'|'u32'|'u64'
 *                    | 'f32'|'f64'|'char'|'void'
 *                    | 'structinstance' | 'defstruct'
 *                    | ARRAY_TYPE_TOKEN   (e.g. "a<i32>")
 *                    | POINTER_TYPE_TOKEN (e.g. "p<char>")
 *
 *   expr           ::= assignment
 *   assignment     ::= logical ('=' logical)*
 *   logical        ::= comparison (('and'|'or') comparison)*
 *   comparison     ::= bitwise (('=='|'!='|'<'|'>'|'<='|'>=') bitwise)*
 *   bitwise        ::= shift (('&'|'|'|'^') shift)*
 *   shift          ::= additive (('<<'|'>>') additive)*
 *   additive       ::= multiplicative (('+'|'-') multiplicative)*
 *   multiplicative ::= unary (('*'|'/'|'%') unary)*
 *   unary          ::= ('!'|'-'|'~') unary | postfix
 *   postfix        ::= primary (call_suffix | index_suffix | field_suffix)*
 *   call_suffix    ::= '(' arg_list ')'
 *   index_suffix   ::= '[' expr ']'
 *   field_suffix   ::= '.' IDENT
 *   primary        ::= INT_LITERAL | FLOAT_LITERAL | FLOAT32_LITERAL
 *                    | CHAR_LITERAL | STRING_LITERAL
 *                    | IDENT | 'new' IDENT
 *                    | '{' expr (',' expr)* '}'   array literal
 *                    | '(' expr ')'
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "parser.h"
#include "AST.h"
#include "../lexer/token.h"
#include "../lexer/vector.h"
#include "../throwErr.h"


/* Helpers */


static Token *parser_peek(Parser *p) {
    return &p->tokens->data[p->pos];
}

static __attribute__((unused)) Token *parser_peek_offset(Parser *p, unsigned int offset) {
    unsigned int idx = p->pos + offset;
    if (idx >= p->tokens->size)
        idx = p->tokens->size - 1; /* clamp to EOF */
    return &p->tokens->data[idx];
}

static Token *parser_advance(Parser *p) {
    Token *t = parser_peek(p);
    if (t->tokenType != Eof)
        p->pos++;
    return t;
}

static bool parser_check(Parser *p, enum TokenType tt) {
    return parser_peek(p)->tokenType == tt;
}

static bool parser_match(Parser *p, enum TokenType tt) {
    if (parser_check(p, tt)) {
        parser_advance(p);
        return true;
    }
    return false;
}

/* Consume token or emit error. Returns the consumed token (or peek on fail). */
static Token *parser_expect(Parser *p, enum TokenType tt, const char *msg) {
    if (parser_check(p, tt))
        return parser_advance(p);

    Token *bad = parser_peek(p);
    fprintf(stderr,
        "\e[31m\e[1m\e[4m\e[40mPARSER ERROR - \e[0m"
        " \e[4m\e[36m\e[40m%s\e[0m"
        " \e[37m\e[40m (got \"%s\")\e[0m"
        "\n\t\e[0m"
        "\e[4mAt line \e[0m"
        "\e[1m\e[33m\e[40m%u\e[0m"
        "\e[4m , and Column \e[0m"
        "\e[1m\e[33m\e[40m%u\e[0m\n",
        msg,
        bad->lexeme ? bad->lexeme : "?",
        bad->line,
        bad->column
    );
    p->had_error = true;
    return bad; /* don't advance – let caller recover */
}

/* Synchronise to next statement boundary after an error. */
static void synchronize(Parser *p) {
    while (!parser_check(p, Eof)) {
        if (parser_peek(p)->tokenType == Semicolon) {
            parser_advance(p); /* eat the ';' */
            return;
        }
        switch (parser_peek(p)->tokenType) {
            case Fn:
            case Let:
            case Return:
            case If:
            case Else:
            case While:
            case Goto:
            case Lbl:
            case Use:
            case Asm:
                return;
            default:
                parser_advance(p);
        }
    }
}


/* is_type_token – true if the current token begins a type */


static bool is_type_token(enum TokenType tt) {
    switch (tt) {
        case I16: case I32: case I64:
        case U16: case U32: case U64:
        case F32: case F64:
        case Char:
        case Void:
        case StructInstance:
        case DefStruct:
        case Array:
        case Pointer:
            return true;
        default:
            return false;
    }
}


/* Forward declarations */

static Ast_node *parse_stmt        (Parser *p);
static Ast_node *parse_let_stmt    (Parser *p);
static Ast_node *parse_fn_def      (Parser *p);
static Ast_node *parse_block       (Parser *p);
static Ast_node *parse_return_stmt (Parser *p);
static Ast_node *parse_if_stmt     (Parser *p);
static Ast_node *parse_goto_stmt   (Parser *p);
static Ast_node *parse_label_stmt  (Parser *p);
 static Ast_node *parse_while_stmt  (Parser *p);
static Ast_node *parse_use_stmt    (Parser *p);
static Ast_node *parse_asm_block   (Parser *p);
static Ast_node *parse_type        (Parser *p);
static Ast_node *parse_expr        (Parser *p);
static Ast_node *parse_assignment  (Parser *p);
static Ast_node *parse_logical     (Parser *p);
static Ast_node *parse_comparison  (Parser *p);
static Ast_node *parse_bitwise     (Parser *p);
static Ast_node *parse_shift       (Parser *p);
static Ast_node *parse_additive    (Parser *p);
static Ast_node *parse_multiplicative(Parser *p);
static Ast_node *parse_unary       (Parser *p);
static Ast_node *parse_postfix     (Parser *p);
static Ast_node *parse_primary     (Parser *p);


/* Public API */


Parser *parser_new(Vector *tokens) {
    Parser *p = malloc(sizeof(Parser));
    p->tokens    = tokens;
    p->pos       = 0;
    p->had_error = false;
    return p;
}

void parser_free(Parser *p) {
    free(p);
}

Ast_node *parse(Parser *p) {
    Ast_node *program = ast_node_new(NODE_PROGRAM, NULL);

    while (!parser_check(p, Eof)) {
        Ast_node *decl = NULL;

        switch (parser_peek(p)->tokenType) {
            case Fn:
                decl = parse_fn_def(p);
                break;
            case Let:
                decl = parse_let_stmt(p);
                break;
            case Use:
                decl = parse_use_stmt(p);
                break;
            default:
                /* Unexpected token at top level; emit error and sync */
                fprintf(stderr,
                    "\e[31m\e[1m\e[4m\e[40mPARSER ERROR - \e[0m"
                    " \e[4m\e[36m\e[40mTop-level declaration\e[0m"
                    " \e[37m\e[40m Unexpected token \"%s\"\e[0m"
                    "\n\t\e[0m"
                    "\e[4mAt line \e[0m"
                    "\e[1m\e[33m\e[40m%u\e[0m"
                    "\e[4m , and Column \e[0m"
                    "\e[1m\e[33m\e[40m%u\e[0m\n",
                    parser_peek(p)->lexeme ? parser_peek(p)->lexeme : "?",
                    parser_peek(p)->line,
                    parser_peek(p)->column
                );
                p->had_error = true;
                synchronize(p);
                continue;
        }

        if (decl)
            ast_add_child(program, decl);
    }

    return program;
}


/* Type node */

static Ast_node *parse_type(Parser *p) {
    Token *tok = parser_peek(p);
    if (!is_type_token(tok->tokenType)) {
        parser_expect(p, I32, "Expected a type");
        p->had_error = true;
        return ast_node_new(NODE_TYPE, tok);
    }
    parser_advance(p);
    return ast_node_new(NODE_TYPE, tok);
}


/* use_stmt ::= 'use' STRING_LITERAL ';' */


static Ast_node *parse_use_stmt(Parser *p) {
    Token *kw = parser_advance(p); /* eat 'use' */
    Ast_node *node = ast_node_new(NODE_USE, kw);

    Token *path = parser_expect(p, StringLiteral, "Expected string literal after 'use'");
    ast_add_child(node, ast_node_new(NODE_STRING_LITERAL, path));

    parser_expect(p, Semicolon, "Expected ';' after use statement");
    return node;
}


/* asm_block ::= 'asm' '{' <any tokens> '}' */


static Ast_node *parse_asm_block(Parser *p) {
    Token *kw = parser_advance(p); /* eat 'asm' */
    Ast_node *node = ast_node_new(NODE_ASM_BLOCK, kw);

    parser_expect(p, LeftBrace, "Expected '{' after 'asm'");

    /* Collect raw tokens (as identifier-like children) until matching '}' */
    int depth = 1;
    while (!parser_check(p, Eof) && depth > 0) {
        if (parser_check(p, LeftBrace))  { depth++; }
        if (parser_check(p, RightBrace)) { depth--; if (depth == 0) break; }
        Token *raw = parser_advance(p);
        ast_add_child(node, ast_node_new(NODE_IDENTIFIER, raw));
    }
    parser_expect(p, RightBrace, "Expected '}' to close 'asm' block");
    return node;
}


/* fn_def ::= 'fn' IDENT '->' type '(' param_list ')' block */


static Ast_node *parse_param(Parser *p) {
    /* name : type */
    Token *name = parser_expect(p, Identifier, "Expected parameter name");
    Ast_node *param = ast_node_new(NODE_PARAM, name);

    parser_expect(p, Colon, "Expected ':' after parameter name");
    Ast_node *type_node = parse_type(p);
    ast_add_child(param, type_node);
    return param;
}

static Ast_node *parse_fn_def(Parser *p) {
    Token *kw   = parser_advance(p); /* eat 'fn' */
    Ast_node *fn = ast_node_new(NODE_FN_DEF, kw);

    /* function name */
    Token *name = parser_expect(p, Identifier, "Expected function name after 'fn'");
    ast_add_child(fn, ast_node_new(NODE_IDENTIFIER, name));

    /* '->' return type */
    parser_expect(p, Arrow, "Expected '->' after function name");
    Ast_node *ret_type = parse_type(p);
    ast_add_child(fn, ret_type);

    /* '(' param_list ')' */
    parser_expect(p, LeftParen, "Expected '(' for parameter list");
    while (!parser_check(p, RightParen) && !parser_check(p, Eof)) {
        Ast_node *param = parse_param(p);
        ast_add_child(fn, param);
        if (!parser_match(p, Comma))
            break;
    }
    parser_expect(p, RightParen, "Expected ')' after parameter list");

    /* body block */
    Ast_node *body = parse_block(p);
    ast_add_child(fn, body);

    return fn;
}


/* block ::= '{' stmt* '}' */


static Ast_node *parse_block(Parser *p) {
    Token *brace = parser_expect(p, LeftBrace, "Expected '{'");
    Ast_node *block = ast_node_new(NODE_PROGRAM, brace); /* reuse PROGRAM as a generic block */

    while (!parser_check(p, RightBrace) && !parser_check(p, Eof)) {
        Ast_node *s = parse_stmt(p);
        if (s)
            ast_add_child(block, s);
    }

    parser_expect(p, RightBrace, "Expected '}'");
    return block;
}


/* let_stmt */
/*   let IDENT : type = expr ; */
/*   let IDENT : defstruct = { field* } ; */


static Ast_node *parse_let_stmt(Parser *p) {
    Token *kw = parser_advance(p); /* eat 'let' */
    Ast_node *node = ast_node_new(NODE_LET, kw);

    Token *name = parser_expect(p, Identifier, "Expected variable name after 'let'");
    ast_add_child(node, ast_node_new(NODE_IDENTIFIER, name));

    parser_expect(p, Colon, "Expected ':' after variable name");

    /* type annotation */
    Ast_node *type_node = parse_type(p);
    ast_add_child(node, type_node);

    /* defstruct body: let Name : defstruct = { field* }; */
    if (type_node->token && type_node->token->tokenType == DefStruct) {
        Ast_node *ds = ast_node_new(NODE_DEF_STRUCT, kw);

        parser_expect(p, Assign, "Expected '=' after 'defstruct'");
        parser_expect(p, LeftBrace, "Expected '{' for struct body");

        while (!parser_check(p, RightBrace) && !parser_check(p, Eof)) {
            /* struct field: 'let' IDENT ':' type ','? */
            if (!parser_check(p, Let)) {
                fprintf(stderr,
                    "\e[31m\e[1m\e[4m\e[40mPARSER ERROR - \e[0m"
                    " \e[4m\e[36m\e[40mstruct field\e[0m"
                    " \e[37m\e[40m Expected 'let' for struct field\e[0m"
                    "\n\t\e[4mAt line \e[0m"
                    "\e[1m\e[33m\e[40m%u\e[0m"
                    "\e[4m , and Column \e[0m"
                    "\e[1m\e[33m\e[40m%u\e[0m\n",
                    parser_peek(p)->line, parser_peek(p)->column);
                p->had_error = true;
                synchronize(p);
                break;
            }
            parser_advance(p); /* eat 'let' */
            Token *fname = parser_expect(p, Identifier, "Expected field name");
            Ast_node *field = ast_node_new(NODE_STRUCT_FIELD, fname);
            parser_expect(p, Colon, "Expected ':' after field name");
            ast_add_child(field, parse_type(p));
            parser_match(p, Comma); /* optional comma */
            ast_add_child(ds, field);
        }
        parser_expect(p, RightBrace, "Expected '}' after struct body");
        parser_expect(p, Semicolon, "Expected ';' after struct definition");
        ast_add_child(node, ds);
        return node;
    }

    /* Optional initialiser */
    if (parser_match(p, Assign)) {
        Ast_node *init = parse_expr(p);
        if (init)
            ast_add_child(node, init);
    }

    parser_expect(p, Semicolon, "Expected ';' after let statement");
    return node;
}


/* Statement dispatcher */


static Ast_node *parse_stmt(Parser *p) {
    switch (parser_peek(p)->tokenType) {
        case Let:    return parse_let_stmt(p);
        case Return: return parse_return_stmt(p);
        case If:     return parse_if_stmt(p);
        case Goto:   return parse_goto_stmt(p);
        case Lbl:    return parse_label_stmt(p);
        case Use:    return parse_use_stmt(p);
        case Asm:    return parse_asm_block(p);
        case While:  return parse_while_stmt(p);
        default:     break;
    }

    /* expression statement */
    Ast_node *expr = parse_expr(p);
    if (!expr) {
        /* couldn't parse anything; skip one token to avoid infinite loop */
        parser_advance(p);
        return NULL;
    }
    Ast_node *stmt = ast_node_new(NODE_EXPR_STMT, expr->token);
    ast_add_child(stmt, expr);
    parser_expect(p, Semicolon, "Expected ';' after expression");
    return stmt;
}


/* return_stmt ::= 'return' expr? ';' */


static Ast_node *parse_return_stmt(Parser *p) {
    Token *kw = parser_advance(p); /* eat 'return' */
    Ast_node *node = ast_node_new(NODE_RETURN, kw);

    if (!parser_check(p, Semicolon)) {
        Ast_node *val = parse_expr(p);
        if (val) ast_add_child(node, val);
    }

    parser_expect(p, Semicolon, "Expected ';' after return value");
    return node;
}


/* if_stmt ::= 'if' '(' expr ')' block ('else' (block | if_stmt))?   */


static Ast_node *parse_if_stmt(Parser *p) {
    Token *kw = parser_advance(p); /* eat 'if' */
    Ast_node *node = ast_node_new(NODE_IF, kw);

    parser_expect(p, LeftParen, "Expected '(' after 'if'");
    Ast_node *cond = parse_expr(p);
    if (cond) ast_add_child(node, cond);
    parser_expect(p, RightParen, "Expected ')' after if condition");

    Ast_node *then_body = parse_block(p);
    ast_add_child(node, then_body);

    /* Optional else / else-if */
    if (parser_match(p, Else)) {
        if (parser_check(p, If)) {
            /* else if — recurse */
            Ast_node *elif = parse_if_stmt(p);
            ast_add_child(node, elif);
        } else {
            /* plain else block */
            Ast_node *else_body = parse_block(p);
            ast_add_child(node, else_body);
        }
    }

    return node;
}


/* while_stmt ::= 'while' '(' expr ')' block */


static Ast_node *parse_while_stmt(Parser *p) {
    Token *kw = parser_advance(p); /* eat 'while' */
    Ast_node *node = ast_node_new(NODE_WHILE, kw);

    parser_expect(p, LeftParen, "Expected '(' after 'while'");
    Ast_node *cond = parse_expr(p);
    if (cond) ast_add_child(node, cond);
    parser_expect(p, RightParen, "Expected ')' after while condition");

    Ast_node *body = parse_block(p);
    ast_add_child(node, body);

    return node;
}


/* goto_stmt  ::= 'goto' IDENT ';' */
/* label_stmt ::= 'lbl'  IDENT ':' */


static Ast_node *parse_goto_stmt(Parser *p) {
    Token *kw = parser_advance(p);
    Ast_node *node = ast_node_new(NODE_GOTO, kw);
    Token *lbl = parser_expect(p, Identifier, "Expected label name after 'goto'");
    ast_add_child(node, ast_node_new(NODE_IDENTIFIER, lbl));
    parser_expect(p, Semicolon, "Expected ';' after goto");
    return node;
}

static Ast_node *parse_label_stmt(Parser *p) {
    Token *kw = parser_advance(p); /* eat 'lbl' */
    Ast_node *node = ast_node_new(NODE_LABEL, kw);
    Token *lbl = parser_expect(p, Identifier, "Expected label name after 'lbl'");
    ast_add_child(node, ast_node_new(NODE_IDENTIFIER, lbl));
    parser_expect(p, Colon, "Expected ':' after label name");
    return node;
}


/* Expression parsing (Pratt-style precedence climbing) */
static Ast_node *parse_expr(Parser *p) {
    return parse_assignment(p);
}

/* assignment ::= logical ('=' logical)* */
static Ast_node *parse_assignment(Parser *p) {
    Ast_node *left = parse_logical(p);

    while (parser_check(p, Assign)) {
        Token *op = parser_advance(p);
        Ast_node *right = parse_logical(p);
        Ast_node *node = ast_node_new(NODE_ASSIGN, op);
        ast_add_child(node, left);
        ast_add_child(node, right);
        left = node;
    }
    return left;
}

/* logical ::= comparison (('and'|'or') comparison)* */
static Ast_node *parse_logical(Parser *p) {
    Ast_node *left = parse_comparison(p);

    while (parser_check(p, And) || parser_check(p, Or)) {
        Token *op = parser_advance(p);
        Ast_node *right = parse_comparison(p);
        Ast_node *node = ast_node_new(NODE_BINARY_OP, op);
        ast_add_child(node, left);
        ast_add_child(node, right);
        left = node;
    }
    return left;
}

/* comparison ::= bitwise (('=='|'!='|'<'|'>'|'<='|'>=') bitwise)* */
static Ast_node *parse_comparison(Parser *p) {
    Ast_node *left = parse_bitwise(p);

    for (;;) {
        enum TokenType tt = parser_peek(p)->tokenType;
        if (tt != Equal && tt != NotEqual &&
            tt != LessThan && tt != GreaterThan &&
            tt != LessEqual && tt != GreaterEqual)
            break;

        Token *op = parser_advance(p);
        Ast_node *right = parse_bitwise(p);
        Ast_node *node = ast_node_new(NODE_BINARY_OP, op);
        ast_add_child(node, left);
        ast_add_child(node, right);
        left = node;
    }
    return left;
}

/* bitwise ::= shift (('&'|'|'|'^') shift)* */
static Ast_node *parse_bitwise(Parser *p) {
    Ast_node *left = parse_shift(p);

    for (;;) {
        enum TokenType tt = parser_peek(p)->tokenType;
        if (tt != BitwiseAnd && tt != BitwiseOr && tt != BitwiseXor)
            break;

        Token *op = parser_advance(p);
        Ast_node *right = parse_shift(p);
        Ast_node *node = ast_node_new(NODE_BINARY_OP, op);
        ast_add_child(node, left);
        ast_add_child(node, right);
        left = node;
    }
    return left;
}

/* shift ::= additive (('<<'|'>>') additive)* */
static Ast_node *parse_shift(Parser *p) {
    Ast_node *left = parse_additive(p);

    while (parser_check(p, LeftShift) || parser_check(p, RightShift)) {
        Token *op = parser_advance(p);
        Ast_node *right = parse_additive(p);
        Ast_node *node = ast_node_new(NODE_BINARY_OP, op);
        ast_add_child(node, left);
        ast_add_child(node, right);
        left = node;
    }
    return left;
}

/* additive ::= multiplicative (('+' | '-') multiplicative)* */
static Ast_node *parse_additive(Parser *p) {
    Ast_node *left = parse_multiplicative(p);

    while (parser_check(p, Plus) || parser_check(p, Minus)) {
        Token *op = parser_advance(p);
        Ast_node *right = parse_multiplicative(p);
        Ast_node *node = ast_node_new(NODE_BINARY_OP, op);
        ast_add_child(node, left);
        ast_add_child(node, right);
        left = node;
    }
    return left;
}

/* multiplicative ::= unary (('*' | '/' | '%') unary)* */
static Ast_node *parse_multiplicative(Parser *p) {
    Ast_node *left = parse_unary(p);

    while (parser_check(p, Star) || parser_check(p, Slash) || parser_check(p, Percent)) {
        Token *op = parser_advance(p);
        Ast_node *right = parse_unary(p);
        Ast_node *node = ast_node_new(NODE_BINARY_OP, op);
        ast_add_child(node, left);
        ast_add_child(node, right);
        left = node;
    }
    return left;
}

/* unary ::= ('!' | '-' | '~') unary | postfix */
static Ast_node *parse_unary(Parser *p) {
    if (parser_check(p, Not) || parser_check(p, Minus) || parser_check(p, BitwiseNot)) {
        Token *op = parser_advance(p);
        Ast_node *operand = parse_unary(p);
        Ast_node *node = ast_node_new(NODE_UNARY_OP, op);
        ast_add_child(node, operand);
        return node;
    }
    return parse_postfix(p);
}

/*
 * postfix ::= primary (call_suffix | index_suffix | field_suffix)*
 *   call_suffix  ::= '(' arg_list ')'
 *   index_suffix ::= '[' expr ']'
 *   field_suffix ::= '.' IDENT
 */
static Ast_node *parse_postfix(Parser *p) {
    Ast_node *node = parse_primary(p);
    if (!node) return NULL;

    for (;;) {
        if (parser_check(p, LeftParen)) {
            /* function call */
            Token *paren = parser_advance(p); /* eat '(' */
            Ast_node *call = ast_node_new(NODE_CALL, paren);
            ast_add_child(call, node);

            /* arguments */
            while (!parser_check(p, RightParen) && !parser_check(p, Eof)) {
                Ast_node *arg = parse_expr(p);
                if (arg) ast_add_child(call, arg);
                if (!parser_match(p, Comma))
                    break;
            }
            parser_expect(p, RightParen, "Expected ')' after arguments");
            node = call;

        } else if (parser_check(p, LeftBracket)) {
            /* array index */
            Token *bracket = parser_advance(p); /* eat '[' */
            Ast_node *idx_node = ast_node_new(NODE_INDEX, bracket);
            ast_add_child(idx_node, node);
            Ast_node *idx = parse_expr(p);
            if (idx) ast_add_child(idx_node, idx);
            parser_expect(p, RightBracket, "Expected ']' after index");
            node = idx_node;

        } else if (parser_check(p, Dot)) {
            /* field access */
            Token *dot = parser_advance(p); /* eat '.' */
            Ast_node *fa = ast_node_new(NODE_FIELD_ACCESS, dot);
            ast_add_child(fa, node);
            Token *field = parser_expect(p, Identifier, "Expected field name after '.'");
            ast_add_child(fa, ast_node_new(NODE_IDENTIFIER, field));
            node = fa;

        } else {
            break;
        }
    }
    return node;
}

/*
 * primary ::= INT_LITERAL | FLOAT_LITERAL | FLOAT32_LITERAL
 *           | CHAR_LITERAL | STRING_LITERAL
 *           | IDENT
 *           | 'new' IDENT
 *           | '{' expr (',' expr)* '}'
 *           | '(' expr ')'
 */
static Ast_node *parse_primary(Parser *p) {
    Token *tok = parser_peek(p);

    switch (tok->tokenType) {
        case IntLiteral:
            parser_advance(p);
            return ast_node_new(NODE_INT_LITERAL, tok);

        case FloatLiteral:
            parser_advance(p);
            return ast_node_new(NODE_FLOAT_LITERAL, tok);

        case Float32Literal:
            parser_advance(p);
            return ast_node_new(NODE_FLOAT32_LITERAL, tok);

        case CharLiteral:
            parser_advance(p);
            return ast_node_new(NODE_CHAR_LITERAL, tok);

        case StringLiteral:
            parser_advance(p);
            return ast_node_new(NODE_STRING_LITERAL, tok);

        case Identifier:
            parser_advance(p);
            return ast_node_new(NODE_IDENTIFIER, tok);

        case New: {
            /* 'new' IDENT  – heap-allocated struct instance */
            parser_advance(p); /* eat 'new' */
            Ast_node *sn = ast_node_new(NODE_STRUCT_NEW, tok);
            Token *struct_name = parser_expect(p, Identifier, "Expected struct name after 'new'");
            ast_add_child(sn, ast_node_new(NODE_IDENTIFIER, struct_name));
            return sn;
        }

        case LeftBrace: {
            /* array literal: { expr, expr, ... } */
            Token *brace = parser_advance(p); /* eat '{' */
            Ast_node *arr = ast_node_new(NODE_ARRAY_LITERAL, brace);
            while (!parser_check(p, RightBrace) && !parser_check(p, Eof)) {
                Ast_node *elem = parse_expr(p);
                if (elem) ast_add_child(arr, elem);
                if (!parser_match(p, Comma))
                    break;
            }
            parser_expect(p, RightBrace, "Expected '}' after array literal");
            return arr;
        }

        case LeftParen: {
            /* grouped expression */
            parser_advance(p); /* eat '(' */
            Ast_node *inner = parse_expr(p);
            parser_expect(p, RightParen, "Expected ')' in grouped expression");
            return inner;
        }

        default:
            /* Not an expression start - report error */
            fprintf(stderr,
                "\e[31m\e[1m\e[4m\e[40mPARSER ERROR - \e[0m"
                " \e[4m\e[36m\e[40mExpression\e[0m"
                " \e[37m\e[40m Unexpected token \"%s\"\e[0m"
                "\n\t\e[0m"
                "\e[4mAt line \e[0m"
                "\e[1m\e[33m\e[40m%u\e[0m"
                "\e[4m , and Column \e[0m"
                "\e[1m\e[33m\e[40m%u\e[0m\n",
                tok->lexeme ? tok->lexeme : "?",
                tok->line,
                tok->column
            );
            p->had_error = true;
            return NULL;
    }
}