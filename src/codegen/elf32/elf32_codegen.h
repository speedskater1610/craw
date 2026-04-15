#ifndef ELF32_Codegen_H
#define ELF32_Codegen_H

#include <stdio.h>
#include <stdbool.h>
#include "../../parser/AST.h"

/*
 * code generator for CRAW.
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
} ELF32_StrBuf;

void ELF32_StrBuf_init   (ELF32_StrBuf *sb);
void ELF32_StrBuf_free   (ELF32_StrBuf *sb);
void ELF32_StrBuf_appendf(ELF32_StrBuf *sb, const char *fmt, ...);
void ELF32_StrBuf_append (ELF32_StrBuf *sb, const char *s);

 
/* String-literal table (for the .data section) */
#define ELF32_Codegen_MAX_STRINGS 256

typedef struct {
    char label[32];     /* unique asm label, e.g. .str0 */
    char *value;        /* raw bytes (already escape-processed) */
    int   len;          /* length including null terminator */
} ELF32_StrEntry;

 
/* Struct type table — collected from NODE_DEF_STRUCT nodes */
#define ELF32_Codegen_MAX_STRUCT_FIELDS 32
#define ELF32_Codegen_MAX_STRUCTS       64

typedef struct {
    char name[64];   /* field name */
    int  offset;     /* byte offset from struct base */
    int  size;       /* bytes */
} ELF32_StructField;

typedef struct {
    char        name[64];
    ELF32_StructField fields[ELF32_Codegen_MAX_STRUCT_FIELDS];
    int         nfields;
    int         total_size;  /* sum of all field sizes, padded to 4 */
} ELF32_StructDef;

typedef struct {
    ELF32_StructDef defs[ELF32_Codegen_MAX_STRUCTS];
    int       count;
} ELF32_StructTable;

 
/* Symbol table */
#define ELF32_Codegen_MAX_LOCALS 256

typedef struct {
    char name[64];
    int  offset;   /* from ebp: negative = local, +8.. = param */
    int  size;     /* bytes */
} ELF32_LocalVar;

typedef struct {
    ELF32_LocalVar vars[ELF32_Codegen_MAX_LOCALS];
    int      count;
    int      next_offset;  /* next free negative offset */
} ELF32_SymTable;

 
/* Code generator state */
typedef struct {
    ELF32_StrBuf   text;          /* .text section buffer */
    ELF32_StrBuf   data;          /* .data section buffer */
    ELF32_StrBuf   pre;           /* function preamble (sub esp, N) */
    ELF32_StrBuf   body;          /* function body instructions */

    ELF32_StrEntry    strings[ELF32_Codegen_MAX_STRINGS];
    int         nstrings;
    ELF32_StructTable structs;

    /* Global variables — allocated as static storage via _craw_bss labels.
       Each global gets a unique label; accesses use [label] memory addressing.
       Stored as: label, name, size_bytes */
#define ELF32_Codegen_MAX_GLOBALS 128
    struct { char label[32]; char name[64]; int size; } globals[ELF32_Codegen_MAX_GLOBALS];
    int      nglobals;

    ELF32_SymTable syms;
    int      label_counter;
    bool     had_error;
    char     cur_fn[64];
} ELF32_Codegen;

 
/* Public API */
ELF32_Codegen *ELF32_Codegen_new(void);
void ELF32_Codegen_free(ELF32_Codegen *cg);

/* Compile a full program AST; fills cg->text and cg->data. */
void ELF32_Codegen_program(ELF32_Codegen *cg, const Ast_node *program);

/* Return a single heap-allocated string: data section + text section.
   Caller must free(). */
char *ELF32_Codegen_get_asm(ELF32_Codegen *cg);

/* Write complete assembly to a FILE* (e.g. for -S flag or debug). */
void ELF32_Codegen_write_asm(ELF32_Codegen *cg, FILE *f);

void ELF32_Codegen_fresh_label(ELF32_Codegen *cg, char *buf, const char *prefix);

#endif /* ELF32_Codegen_H */