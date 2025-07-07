// #include "read.h"

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>


// char* readFile() {
//     //open the file
//     FILE *fptr = fopen(main_file, "r");
//     if (fptr == NULL) {
//         printf("Error opening file\n");
//         return NULL;
//     }

//     fseek(fptr, 0, SEEK_END);
//     long fileSize = ftell(fptr);
//     fseek(fptr, 0, SEEK_SET);


//     //attemp to use a pointer to see if you can allocate memopry
//     char *input_code = malloc(fileSize + 1);
//     if (!input_code) {
//         printf("Memory allocation failed\n");
//         fclose(fptr);
//         return NULL;
//     }


//     //get the file contents into the input code 
//     fread(input_code, 1, fileSize, fptr);
//     input_code[fileSize] = '\0';

//     fclose(fptr);
//     return input_code;
// }
#include "read.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// char* readFile() {
//     //open the file
//     FILE *fptr = fopen(main_file, "r");
//     if (fptr == NULL) {
//         printf("Error opening file\n");
//         return NULL;
//     }

//     fseek(fptr, 0, SEEK_END);
//     long fileSize = ftell(fptr);
//     fseek(fptr, 0, SEEK_SET);


//     //attemp to use a pointer to see if you can allocate memopry
//     char *input_code = (char*)malloc(fileSize + 1);
//     if (!input_code) {
//         printf("Memory allocation failed\n");
//         fclose(fptr);
//         return NULL;
//     }


//     //get the file contents into the input code 
//     fread(input_code, 1, fileSize, fptr);
//     input_code[fileSize] = '\0';

//     fclose(fptr);

//     //DEBUG
//     printf("readFile\n%s", input_code);
//     printf("\n\n\n\n");


//     return input_code;
// }



// char* readFile(const char* filename) {
//     FILE *fptr = fopen(filename, "rb");
//     if (!fptr) {
//         perror("Error opening file");
//         return NULL;
//     }

//     fseek(fptr, 0, SEEK_END);
//     long fileSize = ftell(fptr);
//     rewind(fptr); // same as fseek(fptr, 0, SEEK_SET);

//     char *buffer = malloc(fileSize + 1); // +1 for null terminator
//     if (!buffer) {
//         perror("Memory allocation failed");
//         fclose(fptr);
//         return NULL;
//     }

//     size_t bytesRead = fread(buffer, 1, fileSize, fptr);
//     buffer[bytesRead] = '\0';  // Safe: use actual bytes read, not fileSize blindly

//     fclose(fptr);
//     return buffer;
// }


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
