#include "throwErr.h"

void Err(const char *restrict format, ...) {
    va_list args;
    va_start(args, format);
    
    char *buffer = (char*)malloc(
        strlen(format)*sizeof(char) + 
        5*sizeof(long)
    ); // i never really ass in more than 5 ...'s at once and they  arenormally its but just to be careful longs
        
    vsprintf(buffer, 
        format, 
        args
    );
    perror(buffer);
    free(buffer);
    va_end(args);

    has_at_least_one_error = true;
    printf("\n");
}

