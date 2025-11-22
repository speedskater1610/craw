#include "tag.h"

static void print_help(void) {
    printf("Usage: craw [options] <file>\n");
    printf("Options:\n");
    printf("  -h, --help       Show this help message\n");
    printf("  -v, --version    Show compiler version\n");
    printf("  -d, --debug      Enable debug mode\n");
    printf("\nExample:\n");
    printf("  craw --debug test.c\n");
}

static void print_version(void) {
    printf("craw compiler version 0.0.0\n"); // TODO : update this as time goes on and versions change
}

int get_tag(char *tag, bool *debug_mode, char **input_file) {
    if (strcmp(tag, "-h") == 0 || strcmp(tag, "--help") == 0) {
        print_help();
        return 0;
    } 
    else if (strcmp(tag, "-v") == 0 || strcmp(tag, "--version") == 0) {
        print_version();
        return 0;
    } 
    else if (strcmp(tag, "-d") == 0 || strcmp(tag, "--debug") == 0) {
        *debug_mode = true;
        return 0;
    } 
    else if (tag[0] == '-') {
        fprintf(stderr, "Unknown option: %s\n", tag);
        return 1;
    } 
    else {
        *input_file = tag;
        return 0;
    }
}

