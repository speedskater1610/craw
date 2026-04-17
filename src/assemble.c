#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "assemble.h"

#include "assembler/elf32/assembler.h"
#include "assembler/x86-64/assembler.h"
#include "read.h"

#include "ansiColors.h"

void global_assemble(char *src, char *file_out) {
    // TODO: probably with a tag for target
}