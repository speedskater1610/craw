#include <stdio.h>
#include <stdbool.h>

#include "globals.h"
#include "read.h"
#include "lexer/lexer.h"
#include "tag/tag.h"

bool has_at_least_one_error = false;

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
        printf("[DEBUG] Debug mode enabled.\n");
        printf("[DEBUG] Input file: %s\n", input_file);
    }

    // TODO: Pass input_file to your lexer/reader here
    // read_file(input_file);

    return has_at_least_one_error ? 1 : 0;
}
