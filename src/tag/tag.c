#include "tag.h"
#include "../asniColors.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static void print_help(void) {
    printf("Usage: craw [options] <file>\n");
    printf("Options:\n");
    printf("  -h, --help       Show this help message.\n");
    printf("  -v, --version    Show compiler version.\n");
    printf("  -d, --debug      Enable debug mode.\n");
    printf("  -a, --assemble   The <file> will be considered a assembly file.\n");
    printf("\nExample:\n");
    printf("  craw --debug test.c\n");
}

static void print_version(void) {
    printf("" GREEN_HB "craw" RESET " compiler version " CYAN "0.0.0\n" RESET "\n\t From github"); // TODO : update this as time goes on and versions change
}

#define tag(t, s, f) (strcmp(t, s) == 0 || strcmp(t, f) == 0)


int get_tag(char *tag, bool *debug_mode, bool* is_assembling, char **input_file) {
    if (tag(tag, "-h", "--help")) {
        print_help();
    }
    else if (tag(tag, "-v", "--version")) {
        print_version();
    }
    else if (tag(tag, "-d", "--debug")) {
        *debug_mode = true;
    }
    else if (tag(tag, "-a", "--assemble")) {
        *is_assembling = true;
    }
    else if (tag[0] == '-') {
        fprintf(stderr, "" BH_RED "Unknown option:" RESET " %s\n", tag);
        return 1;
    } 
    else {
        *input_file = tag;
    }

    return 0;
}

