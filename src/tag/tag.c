#include "tag.h"
#include "../ansiColors.h"
#include "../read.h"
#include "../globals.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static void print_help(void) {
    printf("Usage: craw [options] <file>\n");
    printf("Options:\n");
    printf("  -h, --help             Show this help message.\n");
    printf("  -d  --deaper-help      This will print deeper and more complete help.\n");
    printf("  -v, --version          Show compiler version.\n");
    printf("  -d, --debug            Enable debug mode.\n");
    printf("  -a, --assemble         The <file> will be considered a assembly file.\n");
    printf("  -S                     Emit assembly source only; do not assemble.\n");
    printf("  -o, --output <file>    Set the output filename (default: \"out\" or \"out.asm\").\n");
    printf("  -#  --taget-help       Prints help on how to compile to different targets\n");
    printf("\nExample:\n");
    printf("  craw --debug test.craw\n");
}

__attribute__((unused)) static void print_deaper_help(void) {
    // printf("");
}


__attribute__((unused)) static void print_help_assembler(void) {
    // printf("");    
}

static void print_version(void) {
    // get the version 
    // TODO: verify that the version file exists if not prompt to start setup
    char *version = read_file("~/.config/craw/version.txt");
    printf("" GREEN_HB "craw" RESET " compiler version " CYAN "%s\n" RESET "\n\t From github", version); // TODO : update this as time goes on and versions change
}

static void print_target_help() {
    printf("\t--target-elf32\t\tProduces a elf32 binary\n");
    printf("\t--target-x86-64\t\tProduces a x86-64 LLVM object file\n");
}

#define tag(t, s, f) (strcmp(t, s) == 0 || strcmp(t, f) == 0)

int get_tag(char *tag, bool *debug_mode, bool* is_assembling, char **input_file) {
    /* Defualt normal tag things */
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
    else if (tag(tag, "-#", "--taget-help")) {
        print_target_help();
    } 
    /* --- COMPILE TARGET OPTIONS --- */
    else if (strcmp(tag, "--target-elf32") == 0) {
        printf("\t\tTargeting " GREEN_HB "ELF32 binary" RESET);
        arch = ELF32;
    } 
    else if (strcmp(tag, "--target-x86-64") == 0) {
        printf("\t\tTargeting " GREEN_HB "x86-64 llvm object file" RESET);
        arch = X86_64;
    }
    /* Base Case */
    else if (tag[0] == '-') {
        fprintf(stderr, "" BH_RED "Unknown option:" RESET " %s\n", tag);
        return 1;
    }
    else {
        *input_file = tag;
    }

    return 0;
}