#include <stdio.h>
#include <stdbool.h>

#include "globals.h"
#include "read.h"
#include "lexer.h" 

bool has_at_least_one_error = false;

int main () {
    printf("RUNS PROPERLY");
    if (has_at_least_one_error) return 1;
    else return 0;
}