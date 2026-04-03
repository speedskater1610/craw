#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include <stdbool.h>
#include "../parser/AST.h"

/*
 * x86-32 code generator for CRAW.
 *
 * Emits CRASM-syntax assembly targeting the ELF32 C++ backend.
 *
 * Calling convention (cdecl x86-32):
 *   Arguments pushed right-to-left; caller cleans up.
 *   Return value in eax.  ebp/ebx/esi/edi are callee-saved.
 *
 * Stack frame:
 *   [ebp + 8 + 4*n]  nth param (0-indexed)
 *   [ebp]            saved ebp
 *   [ebp - 4*n]      nth local (4-byte slots)
 */


/* Dynamic string buffer (for in-memory assembly output) */

typedef struct {
    char  *data;
    size_t len;
    size_t cap;
} StrBuf;

void strbuf_init   (StrBuf *sb);
void strbuf_free   (StrBuf *sb);
void strbuf_appendf(StrBuf *sb, const char *fmt, ...);
void strbuf_append (StrBuf *sb, const char *s);


/* String-literal table (for the .data section) */

#define CODEGEN_MAX_STRINGS 256

typedef struct {
    char label[32];     /* unique asm label, e.g. .str0 */
    char *value;        /* raw bytes (already escape-processed) */
    int   len;          /* length including null terminator */
} StrEntry;


/* Struct type table — collected from NODE_DEF_STRUCT nodes */

#define CODEGEN_MAX_STRUCT_FIELDS 32
#define CODEGEN_MAX_STRUCTS       64

typedef struct {
    char name[64];   /* field name */
    int  offset;     /* byte offset from struct base */
    int  size;       /* bytes */
} StructField;

typedef struct {
    char        name[64];
    StructField fields[CODEGEN_MAX_STRUCT_FIELDS];
    int         nfields;
    int         total_size;  /* sum of all field sizes, padded to 4 */
} StructDef;

typedef struct {
    StructDef defs[CODEGEN_MAX_STRUCTS];
    int       count;
} StructTable;


/* Symbol table */

#define CODEGEN_MAX_LOCALS 256

typedef struct {
    char name[64];
    int  offset;   /* from ebp: negative = local, +8.. = param */
    int  size;     /* bytes */
} LocalVar;

typedef struct {
    LocalVar vars[CODEGEN_MAX_LOCALS];
    int      count;
    int      next_offset;  /* next free negative offset */
} SymTable;


/* Code generator state                                                 */

typedef struct {
    StrBuf   text;          /* .text section buffer           */
    StrBuf   data;          /* .data section buffer           */
    StrBuf   pre;           /* function preamble (sub esp, N) */
    StrBuf   body;          /* function body instructions     */

    StrEntry    strings[CODEGEN_MAX_STRINGS];
    int         nstrings;
    StructTable structs;

    /* Global variables — allocated as static storage via _craw_bss labels.
       Each global gets a unique label; accesses use [label] memory addressing.
       Stored as: label, name, size_bytes */
#define CODEGEN_MAX_GLOBALS 128
    struct { char label[32]; char name[64]; int size; } globals[CODEGEN_MAX_GLOBALS];
    int      nglobals;

    SymTable syms;
    int      label_counter;
    bool     had_error;
    char     cur_fn[64];
} Codegen;


/* Public API                                                           */

Codegen *codegen_new   (void);
void     codegen_free  (Codegen *cg);

/* Compile a full program AST; fills cg->text and cg->data. */
void     codegen_program(Codegen *cg, const Ast_node *program);

/* Return a single heap-allocated string: data section + text section.
   Caller must free(). */
char    *codegen_get_asm(Codegen *cg);

/* Write complete assembly to a FILE* (e.g. for -S flag or debug). */
void     codegen_write_asm(Codegen *cg, FILE *f);

void     codegen_fresh_label(Codegen *cg, char *buf, const char *prefix);

#endif /* CODEGEN_H */