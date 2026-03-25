#include "write.h"

void write_file(char *path, char *data) {
    FILE *fptr = fopen(path, "w");

    if (fptr == NULL) {
        printf("SYSTEM error: opening file!");
        return;
    }

    fprintf(fptr, "%s", data);

    fclose(fptr);
    return;
}
