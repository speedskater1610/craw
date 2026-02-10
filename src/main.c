#include <stdio.h>
#include <stdbool.h>

#include "globals.h"
#include "read.h"
#include "tag/tag.h"
#include "ansiColors.h"
#include "preprocess/preprocessor.h"
#include "lexer/lexer.h"
#include "lexer/vector.h"

bool has_at_least_one_error = false;
bool debug_mode_enables = false;

int main(int argc, char *argv[]) {
    bool debug_mode = false;
    char *input_file = NULL;

    for (int i = 1; i < argc; i++) {
        int result = get_tag(argv[i], &debug_mode, &input_file);
        if (result != 0)
            return result; 
    }

    if (input_file == NULL) {
        fprintf(stderr, "Error: No input file specified. Use --help for usage.\n");
        return 1;
    }

    if (debug_mode) {
        printf( B_RED "[DEBUG]" RESET B_CYAN " Debug mode enabled.\n" RESET);
        printf(B_RED "[DEBUG]" RESET B_CYAN " Input file: %s\n" RESET, input_file);
        debug_mode_enables = true;
    }

    char *src = read_file(input_file);
    
    if (!src) {
        fprintf(stderr, "Error: Cannot read input file.\n");
        return 1;
    }

    char *processed = preprocess(src);

    // pass it into the lexer to get a vector of tokens
    if (debug_mode) {
        printf("%s\n", processed);
    }
    Lexer* lexer = Lexer_new(processed);
    Vector tokens = tokenize(lexer);

    
    free(src);
    free_preprocessed(processed);
    vector_free(&tokens); 
    
    return has_at_least_one_error ? 1 : 0;
}
