#include <stdio.h>
#include <stdbool.h>

#include "globals.h"
#include "read.h"
#include "lexer/lexer.h" 

bool has_at_least_one_error = false;

int main (int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        printf("%s", argv[i]);
    }
    
    if (has_at_least_one_error) return 1;
    else return 0;
}
