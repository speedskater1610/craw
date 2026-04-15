#ifndef GLOBAL_H
#define GLOBAL_H

#include "codegen/arch.h"
#include <stdbool.h>

extern bool has_at_least_one_error;
extern bool debug_mode_enables;
extern bool is_assembling;
extern Arch arch;

#endif  // GLOBAL_H
