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

/*when throwing a error msg use these colors & format to help the user see what is what

ex.
Lexer Error: invalid f32, No conversion could be performed.\n\tAt line %d, annd Column %d\n
error type      msg after      final msg more descript             where to find
    red            blue               base                           nums are orange
    
Err (
    "\e[31m\e[1m\e[4m\e[40mERROR ERROR - \e[0m" 
    " \e[4m\e[36m\e[40mmsg after\e[0m" 
    " \e[37m\e[40m final msg\e[0m" 
    "\n\t\e[0m" 
    "\e[4mAt line \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m" 
    "\e[4m , and Column \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m\n",
    );

*/