#ifndef READ_H
#define READ_H


#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 

char* read_file(char* filePath) {
    FILE *fptr;
    int ch;
    char* fileContents = NULL;
    long fileSize = 0;
    long currentPos = 0;

    fptr = fopen(filePath, "r"); // read mode
    if (fptr == NULL) {
        perror("Error opening file");
        return NULL; // Null means error
    }


    // Determine file size to allocate memory
    fseek(fptr, 0, SEEK_END);
    fileSize = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);

    // Allocate memory for file contents & null terminator
    fileContents = (char*)malloc(fileSize + 1);
    
    // check for error allocating memory for file contents
    if (fileContents == NULL) {
        perror("Error allocating memory");
        fclose(fptr);
        return NULL;
    }

    // read the file character by character
    while ((ch = fgetc(fptr)) != EOF) {
        fileContents[currentPos++] = (char)ch;
    }

    // null-terminate the string
    fileContents[currentPos] = '\0';

    fclose(fptr);
    return fileContents; 
}

#endif // READ_H
