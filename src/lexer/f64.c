#include "f64.h"

bool check_f64 (const char *str_f64, 
                unsigned int start_line, 
                unsigned int start_column) {
    char *endptr;
    double value = strtod(str_f64, &endptr);

    // check for errors
    if (endptr == str_f64) {
        // no conversion could be performed
        Err (
    "\e[31m\e[1m\e[4m\e[40mLEXER (type) ERROR - \e[0m" 
    " \e[4m\e[36m\e[40minvalid f64,\e[0m" 
    " \e[37m\e[40m No conversion could be preformed.\e[0m" 
    "\n\t\e[0m" 
    "\e[4mAt line \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m" 
    "\e[4m , and Column \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m\n",
        start_line, start_column
        );
        return false;
    } 
    
    if (*endptr != '\0') {
        // convertion was preformed succsefully yet there where unleft chars
        Err (
    "\e[31m\e[1m\e[4m\e[40mLEXER (type) ERROR: Partial conversion - \e[0m" 
    " \e[4m\e[36m\e[40minvalid f64,\e[0m" 
    " \e[37m\e[40m extra characters at: %s\e[0m" 
    "\n\t\e[0m" 
    "\e[4mAt line \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m" 
    "\e[4m , and Column \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m\n",
        endptr, start_line, start_column
        );
        return false;
    }

    /* range errors (overflow/underflow) */
    if (errno == ERANGE) {
        Err(
            "\e[31m\e[1m\e[4m\e[40mLEXER (type) ERROR - \e[0m"
            " \e[4m\e[36m\e[40minvalid f32 (out of range),\e[0m"
            " \e[37m\e[40m Value out of range for f32\e[0m"
            "\n\t\e[0m"
            "\e[4mAt line \e[0m"
            "\e[1m\e[33m\e[40m%d\e[0m"
            "\e[4m , and Column \e[0m"
            "\e[1m\e[33m\e[40m%d\e[0m\n",
            start_line, start_column
        );
        return false;
    }

    // Everything looks good
    (void)value; /* NOTE:  suppress unused warning if you don't use value elsewhere */

    return true;
}
