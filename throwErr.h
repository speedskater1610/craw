#ifndef THROW_ERR_H
#define THROW_ERR_H


#include <stdarg.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#include "globals.h"


void Err(const char *restrict format, ...);

#endif  // THROW_ERR_H