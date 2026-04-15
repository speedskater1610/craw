#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "assembler/elf32/assembler.h"
#include "assembler/x86-64/assembler.h"
#include "read.h"

#include "ansiColors.h"

void global_assemble(char *src, char *file_out);
void global_assemble_WIN(char *src, char *file_out);

#endif // ASSEMBLE_H