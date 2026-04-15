#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "globals.h"
#include "read.h"
#include "tag/tag.h"
#include "ansiColors.h"
#include "preprocess/preprocessor.h"
#include "lexer/lexer.h"
#include "lexer/vector.h"
#include "assemble.h"
#include "parser/parser.h"
#include "parser/AST.h"
#include "codegen/codegen.h"

bool has_at_least_one_error = false;
bool debug_mode_enables     = false;
bool is_assembling          = false;

int main(int argc, char *argv[]) {
    bool  debug_mode   = false;
    bool  emit_asm     = false;   /* -S: write .asm only, don't assemble */
    char *input_file   = NULL;
    char *output_file  = NULL;    /* -o <file> */

    for (int i = 1; i < argc; i++) {
        /* Handle -o <file> */
        if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -o requires an output filename.\n");
                return 1;
            }
            output_file = argv[++i];
            continue;
        }
        /* -S: emit assembly source, don't assemble */
        if (strcmp(argv[i], "-S") == 0) {
            emit_asm = true;
            continue;
        }

        int result = get_tag(argv[i], &debug_mode, &is_assembling, &input_file);
        if (result != 0) return result;
    }

    if (input_file == NULL) {
        fprintf(stderr, "Error: No input file specified. Use --help for usage.\n");
        return 1;
    }

    /* Default output filenames */
    if (output_file == NULL)
        output_file = emit_asm ? "out.asm" : "out";

    if (debug_mode) {
        printf(B_RED "[DEBUG]" RESET B_CYAN " Debug mode enabled.\n" RESET);
        printf(B_RED "[DEBUG]" RESET B_CYAN " Input:  %s\n" RESET, input_file);
        printf(B_RED "[DEBUG]" RESET B_CYAN " Output: %s\n" RESET, output_file);
        debug_mode_enables = true;
    }

    char *src = read_file(input_file);
    if (!src) {
        fprintf(stderr, "Error: Cannot read input file '%s'.\n", input_file);
        return 1;
    }

    /* Assembler-only mode (-a flag) */
    if (is_assembling) {
#ifdef _WIN32
        global_assemble_WIN(src, output_file);
#else
        global_assemble(src, output_file);
#endif
        free(src);
        return has_at_least_one_error ? 1 : 0;
    }

    /* Compiler pipeline: preprocess -> lex -> parse -> codegen -> assemble */
    /* Preprocess */
    char *processed = preprocess(src);
    if (!processed) {
        fprintf(stderr, "Error: Preprocessing failed.\n");
        free(src);
        return 1;
    }

    if (debug_mode) {
        printf(B_RED "[DEBUG]" RESET " Preprocessed source:\n%s\n", processed);
    }

    /* Lex */
    Lexer  *lexer  = Lexer_new(processed);
    Vector  tokens = tokenize(lexer);

    /* Parse */
    Parser   *parser = parser_new(&tokens);
    Ast_node *ast    = parse(parser);

    if (debug_mode) {
        printf(B_RED "[DEBUG]" RESET " AST:\n");
        ast_print(ast, 0);
        printf("\n");
    }

    if (parser->had_error) {
        has_at_least_one_error = true;
        fprintf(stderr, "Error: Parse failed — not assembling.\n");
        goto cleanup;
    }

    Arch arch = ELF32;

    /* Codegen */
    {
        switch (arch) {
            case ELF32:
                ELF32_Codegen *cg = ELF32_Codegen_new();
                ELF32_Codegen_program(cg, ast);

                if (cg->had_error) {
                    has_at_least_one_error = true;
                    fprintf(stderr, "Error: Code generation failed.\n");
                    ELF32_Codegen_free(cg);
                    goto cleanup;
                }

                if (debug_mode) {
                    printf(B_RED "[DEBUG]" RESET " Generated assembly:\n");
                    ELF32_Codegen_write_asm(cg, stdout);
                    printf("\n");
                }

                /* -S mode: write .asm file only */
                if (emit_asm) {
                    FILE *asm_f = fopen(output_file, "w");
                    if (!asm_f) {
                        fprintf(stderr, "Error: Cannot open '%s' for writing.\n", output_file);
                        has_at_least_one_error = true;
                    } else {
                        ELF32_Codegen_write_asm(cg, asm_f);
                        fclose(asm_f);
                        printf("Assembly written to %s\n", output_file);
                    }
                } else {
                    /* Normal mode: pipe asm string into the C++ assembler */
                    char *asm_str = ELF32_codegen_get_asm(cg);

                    if (debug_mode)
                        printf(B_RED "[DEBUG]" RESET " Assembling to '%s'...\n", output_file);

                    bool ok = assemble_from_string(asm_str, output_file);
                    if (!ok) {
                        fprintf(stderr, "Error: Assembly failed.\n");
                        has_at_least_one_error = true;
                    } else {
                        printf("Binary written to %s\n", output_file);
                    }
                    free(asm_str);
                }

                ELF32_Codegen_free(cg);
                break;
            case X86_64:
                break;
        }
    }

cleanup:
    ast_free(ast);
    parser_free(parser);
    free(src);
    free_preprocessed(processed);
    vector_free(&tokens);

    return has_at_least_one_error ? 1 : 0;
}
