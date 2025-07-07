#include "write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void writeFile(const char *text) {
    if (text == NULL) {
        fprintf(stderr, "writeFile: NULL input text\n");
        return;
    }

    //length of input text
    size_t len = strlen(text);

    //allocate buffer with 1 extra byte for null terminator
    char *buffer = (char*) malloc(len + 1);
    if (!buffer) {
        perror("Memory allocation failed");
        return;
    }

    //copy
    memcpy(buffer, text, len);

    //null terminate
    buffer[len] = '\0';

    FILE *file = fopen(output_file, "w");
    if (!file) {
        perror("Error opening file");
        free(buffer);
        return;
    }


    //write
    fputs(buffer, file);

    fclose(file);
    free(buffer);
}
