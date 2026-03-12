#ifndef RASSEMBLER_H
#define RASSEMBLER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Assemble x86 AT&T-syntax source code into an LLVM object file.
 *
 * @param source    Null-terminated string of x86 assembly source.
 * @param filename  Null-terminated path for the output object file (e.g. "out.o").
 * @return          NULL on success, or a heap-allocated null-terminated error
 *                  string on failure.  The caller must free the error string
 *                  by passing it to assemble_free_error().
 */
char *assemble(const char *source, const char *filename);

/**
 * Free an error string returned by assemble().
 * Safe to call with NULL.
 */
void assemble_free_error(char *error);

#ifdef __cplusplus
}
#endif

#endif /* RASSEMBLER_H */
