#include <stdio.h>
#include <stdlib.h>

int main() {
    const char *command = "g++ -O2 main.c read/read.c write/write.c builder/build.c std_compile/std.cpp -o craw.exe";

    int result = system(command);

    if (result == 0) {
        printf("build successful! Output: craw.exe\n");
    } else {
        printf("build failed with code:\n\t %d\n", result);
    }

    return result;
}
