
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




#endif /* CODEGEN_H */