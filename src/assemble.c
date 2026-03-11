// BTW no one was ever ment to read this file so go on if you dare

#include <stdio.h>
#include <stdlib.h>

#include "assemble.h"

#include "assembler/mainAssemblerC.h"
#include "assembler/rassembler.h"
#include "read.h"

#include "ansiColors.h"

static int check_assembler() {;
    return atoi(read_file("~/.config/craw/which_assembler.bin"));
}

void global_assemble_WIN(char *src, char *file_out) {
    global_assemble(src, file_out);
}

void global_assemble(char *src, char *file_out) {
    // 0 is the C++ assembler
    // 1 is the new rust assembler
    // I love magic numbers!
    // I should make a a terribly optimized game and make shitty twitch streams using buzzwords
    switch(check_assembler()) {
        case 0:
            assemble_from_string(src, file_out);
            break;

        case 1: // tech expiremental rn
            char *error = assemble(src, file_out);
            if (error != NULL) {
                printf(B_RED "%s" RESET, error);
                assemble_free_error(error);
            }
            break;
    }
}