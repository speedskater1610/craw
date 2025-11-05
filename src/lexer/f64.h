#ifndef F_64_h
#define F_64_h

#include <stdlib.h>
#include <stdbool.h>
#include "../throwErr.h"

// TODO: add type size checks
bool check_f64 (const char *str_f64, 
                unsigned int start_line, 
                unsigned int start_column);

#endif // F_64_h
