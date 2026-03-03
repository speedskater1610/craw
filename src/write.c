#include "write.h"

void write(char *path, char *data) {
    FILE *fptr = fopen("path", "w");

    if (fptr == NULL) {
        printf("SYSTEM error: opening file!");
        return;
    }

    fprintf(fptr, data);
    fclose(fptr);
    return;
}