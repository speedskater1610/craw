#include "f32.h"


bool check_f32 (const char *str_f32,
                unsigned int start_line,
                unsigned int start_column) {
    char *endptr = NULL;
    errno = 0; /* clear errno before call */

    float value = strtof(str_f32, &endptr);

    /* no conversion performed */
    if (endptr == str_f32) {
        Err(
            "\e[31m\e[1m\e[4m\e[40mLEXER (type) ERROR - \e[0m"
            " \e[4m\e[36m\e[40minvalid f32,\e[0m"
            " \e[37m\e[40m No conversion could be performed.\e[0m"
            "\n\t\e[0m"
            "\e[4mAt line \e[0m"
            "\e[1m\e[33m\e[40m%d\e[0m"
            "\e[4m , and Column \e[0m"
            "\e[1m\e[33m\e[40m%d\e[0m\n",
            start_line, start_column
        );
        return false;
    }

    /* trailing junk after the number */
    if (*endptr != '\0') {
        Err(
            "\e[31m\e[1m\e[4m\e[40mLEXER (type) ERROR: Partial conversion - \e[0m"
            " \e[4m\e[36m\e[40minvalid f32,\e[0m"
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

    /* Everything looks good */
    (void)value; /* suppress unused warning if you don't use value elsewhere */
    return true;
}