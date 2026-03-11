#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#include <stdio.h>
#include <stdlib.h>

#include "assemble.h"

#include "assembler/mainAssemblerC.h"
#include "assembler/rassembler.h"
#include "read.h"

#include "ansiColors.h"

void global_assemble(char *src, char *file_out);
void global_assemble_WIN(char *src, char *file_out);

#endif // ASSEMBLE_H