#ifndef F_32_h
#define F_32_h

#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <float.h> 
#include <limits.h>
#include <stdio.h>
#include "../throwErr.h"

// TODO: add type size checks
bool check_f32 (const char *str_f32, 
                unsigned int start_line, 
                unsigned int start_column);

#endif // F_32_h
