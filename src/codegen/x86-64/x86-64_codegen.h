
#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include <stdbool.h>
#include "../../parser/AST.h"

/*
 * another code gen for craw (specifically for the x86-64 assembler).
 *
 * Emits assembly targeting the x86-64 (LLVM) - Rust backend.
 *
 * cdecl x86-64 -- doesnt exist :sob: - :(
 *
 * Calling convention:
 *   Arguments pushed right-to-left; caller should clean up.
 *   Return value in rax.
 *
 */

 
/* Emit helpers - write into a given x86_64_StrBuf */
#define EMIT(buf, ...)   x86_64_StrBuf_appendf((buf), __VA_ARGS__)
#define EMITL(buf, ...)  do { x86_64_StrBuf_appendf((buf), __VA_ARGS__); x86_64_StrBuf_append((buf), "\n"); } while(0)

/* Current function body target */
#define B(cg)  (&(cg)->body)

/* Data section target */
#define D(cg)  (&(cg)->data)

/* Text section (for function labels etc.) */
#define T(cg)  (&(cg)->text)


/* Struct type table - collected from NODE_DEF_STRUCT nodes */
#define x86_64_Codegen_MAX_STRUCT_FIELDS 64
#define x86_64_Codegen_MAX_STRUCTS       128

typedef struct {
    char    name[64];   /* field name */
    int     offset;     /* byte offset from struct base */
    int     size;       /* in bytes */
} x86_64_StructField;

typedef struct {
    char                name[64];
    x86_64_StructField  fields[x86_64_Codegen_MAX_STRUCT_FIELDS];
    int                 nfields;
    int                 total_size;  /* sum of all field sizes, padded to 4 */
} x86_64_StructDef;

typedef struct {
    x86_64_StructDef defs[x86_64_Codegen_MAX_STRUCTS];
    int              count;
} x86_64_StructTable;

/* Symbol table */
#define x86_64_Codegen_MAX_LOCALS 512
typedef struct {
    char name[64];
    int  offset;   /* from ebp: negative = local, +8.. = param */
    int  size;     /* bytes */
} x86_64_LocalVar;

typedef struct {
    x86_64_LocalVar  vars[x86_64_Codegen_MAX_LOCALS];
    int              count;
    int              next_offset;  /* next free negative offset */
} x86_64_SymTable;


typedef struct {
    x86_64_StrBuf   text;   /* .text section buffer */
    x86_64_StrBuf   data;   /* .data section buffer */
    x86_64_StrBuf   pre;    /* function preamble (sub esp, N) */
    x86_64_StrBuf   body;   /* function body instructions */

    x86_64_StrEntry     strings[x86_64_Codegen_MAX_STRINGS];
    int                nstrings;
    x86_64_StructTable  structs;

    /* Global variables allocated as static storage via _craw_bss labels.
       Each global gets a unique label; accesses use [label] memory addressing.
       Stored as: label, name, size_bytes */
#define x86_64_Codegen_MAX_GLOBALS 256
   struct { 
      char label[32]; char name[64]; int size; 
   } globals[x86_64_Codegen_MAX_GLOBALS];
   int      nglobals;

   x86_64_SymTable    syms;
   int                label_counter;
   bool               had_error;
   char               cur_fn[64];
} x86_64_Codegen;


/* Public API */
x86_64_Codegen *x86_64_Codegen_new(void);
void x86_64_Codegen_free();

/* Compile a full program AST; fills cg->text and cg->data. */
void x86_64_Codegen_program();

/* Return a single heap-allocated string: data section + text section.
   Caller must free(). */
char *x86_64_Codegen_get_asm();

/* Write complete assembly to a FILE* (e.g. for -S flag or debug). */
void x86_64_Codegen_write_asm();

#endif /* CODEGEN_H */