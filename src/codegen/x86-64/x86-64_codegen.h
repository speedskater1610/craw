
#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include <stdbool.h>
#include "../../parser/AST.h"

/*
 * another code gen for craw (specifically for the x86-64 assembler).
 *
 * Emits rassembler-syntax assembly targeting the x86-64 (LLVM) - Rust backend.
 *
 * Calling convention (cdecl x86-64):
 *   Arguments pushed right-to-left; caller should clean up.
 *   Return value in rax.  rbp/rbx/rsi/rdi are callee-saved.
 *
 * Stack frame:
 *   [rbp + 8 + 4*n]  nth param (0-indexed)
 *   [rbp]            saved ebp
 *   [rbp - 4*n]      nth local (4-byte slots)
 */

typedef struct {} x86_64_Codegen;

/* NOTE: Still decided if I want to rewrie elf32 codegen or write a new one */

/* Public API */
x86_64_Codegen *x86_64_Codegen_new();
void x86_64_Codegen_free();

/* Compile a full program AST; fills cg->text and cg->data. */
void x86_64_Codegen_program();

/* Return a single heap-allocated string: data section + text section.
   Caller must free(). */
char *x86_64_Codegen_get_asm();

/* Write complete assembly to a FILE* (e.g. for -S flag or debug). */
void x86_64_Codegen_write_asm();

#endif /* CODEGEN_H */