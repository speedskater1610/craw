#include "tag.h"
#include "../ansiColors.h"
#include "../read.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static void print_help(void) {
    printf("Usage: craw [options] <file>\n");
    printf("Options:\n");
    printf("  -h, --help             Show this help message.\n");
    printf("  -d  --deaper-help      This will print deeper and more complete help.\n")
    printf("  -v, --version          Show compiler version.\n");
    printf("  -d, --debug            Enable debug mode.\n");
    printf("  -a, --assemble         The <file> will be considered a assembly file.\n");
    printf("  -r  --rassemble        This will use the rust assembler to assemble <file>.\n")
    printf("  -s  --switch-assembler This will use the none defualt assembler and make that the main on you system.\n")
    printf("\nExample:\n");
    printf("  craw --debug test.craw\n");
}

static void print_deaper_help(void) {
    printf("");
}

static void print_help_assembler(void) {
    printf("");    
}

static void print_version(void) {
    // get the version 
    char *version = read_file("~/.config/craw/version.txt");
    printf("" GREEN_HB "craw" RESET " compiler version " CYAN "" version "\n" RESET "\n\t From github"); // TODO : update this as time goes on and versions change
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