#include "read.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char* readFile() {

    const char* filename = "main_file";
    FILE *fptr = fopen(main_file, "rb");
    if (!fptr) {
        perror("Error opening file");
        return nullptr;
    }

    fseek(fptr, 0, SEEK_END);
    long fileSize = ftell(fptr);
    rewind(fptr);

    // C++ requires casting malloc
    char *buffer = (char*) malloc(fileSize + 1);  // +1 for null terminator
    if (!buffer) {
        perror("Memory allocation failed");
        fclose(fptr);
        return nullptr;
    }

    size_t bytesRead = fread(buffer, 1, fileSize, fptr);
    buffer[bytesRead] = '\0';  // Null-terminate safely

    fclose(fptr);
    return buffer;
}

