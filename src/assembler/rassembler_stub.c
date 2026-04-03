/*
 * rassembler_stub.c
 *
 * Stub implementation of the Rust assembler C FFI.
 * Used when building without the Rust/LLVM backend.
 * The C++ assembler (assemble_from_string) is the active backend.
 */
#include "rassembler.h"
#include <stdlib.h>

char *assemble(const char *source, const char *filename) {
    (void)source; (void)filename;
    return NULL; /* success — but this path is never reached with assembler=0 */
}

void assemble_free_error(char *error) {
    free(error);
}