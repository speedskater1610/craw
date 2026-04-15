#ifndef TAG_H
#define TAG_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "../ansiColors.h"
#include "../read.h"
#include "../globals.h"

int get_tag(char *tag, bool *debug_mode, bool* is_assembling, char **input_file);

#endif // TAG_H
