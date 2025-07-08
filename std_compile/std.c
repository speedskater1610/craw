#include "std.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// WARNING/MESSAGE this file is never used and is just old code that I wrote but was to afraid to give up on and delete


// helper fucntion
char **split_string(const char *str, char delim, int *count) {
    if (!str) {
        *count = 0;
        return NULL;
    }

    // find out how many totoal parts there will be
    int parts = 1;
    for (const char *p = str; *p; p++) {
        if (*p == delim) parts++;
    }

    char **result = malloc(parts * sizeof(char *));
    if (!result) {
        *count = 0;
        return NULL;
    }

    int idx = 0;
    const char *start = str;
    const char *p = str;
    while (*p) {
        if (*p == delim) {
            size_t len = p - start;
            result[idx] = malloc(len + 1);
            if (!result[idx]) {
                // free previous allocations on error
                for (int j = 0; j < idx; j++) free(result[j]);
                free(result);
                *count = 0;
                return NULL;
            }
            strncpy(result[idx], start, len);
            result[idx][len] = '\0';
            idx++;
            start = p + 1;
        }
        p++;
    }
    // final substring after last delimiter
    size_t len = p - start;
    result[idx] = malloc(len + 1);
    if (!result[idx]) {
        for (int j = 0; j < idx; j++) free(result[j]);
        free(result);
        *count = 0;
        return NULL;
    }
    strncpy(result[idx], start, len);
    result[idx][len] = '\0';

    *count = parts;
    return result;
}

// compile the  split lines one by one 
char *compile_data (char* crawData) {
    char *asm_text = craw_text; // not compile now just outline ther structure

    return asm_text;
}


char *compile_start (char* crawBody)  {

}


// the acccual function compioling the code into nasm x86 ia32
// becasue it is a lot simpler to just write 32 bit asm 
char* std_compile(char *input) {
    int count, i;
    char **lines = split_string(input, ';', &count);
    if (!lines) {
        printf("ERROR - failed to split string\n");
        return NULL;
    }

    size_t buf_size = 1024;
    char *crawOut = malloc(buf_size);
    if (!crawOut) {    
        printf("ERROR - memory allocation failed\n");
        for (i = 0; i < count; i++) free(lines[i]);
        free(lines);
        return NULL;
    }
    crawOut[0] = '\0';  // start with empty string so there arent random strings of memeory in the begining

    for (i = 0; i < count; i++) {
        printf("Line %d: \"%s\"\n", i + 1, lines[i]);
        char *compiled = compile(lines[i]); 

        size_t needed = strlen(crawOut) + strlen(compiled) + 2; // +1 for '\n', +1 for '\0'
        if (needed > buf_size) {
            buf_size = needed * 2;
            char *tmp = realloc(crawOut, buf_size);
            if (!tmp) {
                printf("Memory allocation failed\n");
                free(crawOut);
                for (int j = i; j < count; j++) free(lines[j]);
                free(lines);
                return NULL;
            }
            crawOut = tmp;
        }

        strcat(crawOut, compiled);
        strcat(crawOut, "\n");  // separate lines

        // yay the 8th memorey leak i have had to solve in total and the 4th this week
        free(lines[i]);
    }
    free(lines);

    printf("%s \n----- ----- -----\ngets compiled to\n%s\n----- ----- -----", input, crawOut);
    return crawOut;
}





/* DATA VARIBLE TYPES
size_b - db = 1 byte (8-bit)
size_w - dw = 2 bytes (16-bit)
size_d - dd = 4 bytes (32-bit)
*/


/* HELLO WORLD
#include <printf>;

data {
    size_b hello = ("hello, world");
}

start {
    write[hello];
}

===== ===== ===== COMPILES TO

section .data
    hello db "Hello, World!", 10, 0

section .text
    global main
    extern printf

main:
    push ebp
    mov ebp, esp

    push hello
    call printf 
    add esp, 4

    mov esp, ebp
    pop ebp
    ret

*/         
