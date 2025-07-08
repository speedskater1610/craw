// #include <stdlib.h>
// #include <stdio.h>
// #include "build.h"

// void delete_file(char *fileName) {
//     if (remove(fileName) == 0) {
//         // file was successfully removed
//     } else {
//         perror("Error deleting obj in asm config process file");
//     }
// }

// void build_exe_from_asm() { 
//     int result;
//     char cmd[512];

//     // build nasm command string
//     // the out put file is the asm file it is the output of the std_compile in std.c
//     // nasm -f win64 yourfile.asm -o yourfile.obj
//     snprintf(cmd, sizeof(cmd), "nasm -f win64 \"%s\" -o hold.obj", output_file);
//     result = system(cmd);

//     // ERROR handling
//     if (result != 0) {
//         printf("ERROR - NASM (CRAW - ASM) failed with code %d\n", result);
//         return;
//     }

//     result = system("gcc hold.obj -o final.exe");
//     if (result != 0) {
//         printf("ERROR - GCC (ASM - BIN) failed with code %d\n", result);
//         return;
//     }

//     delete_file("hold.obj");
//     //period means it ran fine
//     printf(".\n");
// }
// 


#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include "build.h"

void delete_file(const char *fileName) {
    if (remove(fileName) == 0) {
        // file was successfully removed
    } else {
        perror("Error deleting obj in asm config process file");
    }
}

void build_exe_from_asm(const char *exeFile) { 
    int nasm_result;


    char nasm[512];

    // build nasm command string
    // the out put file is the asm file it is the output of the std_compile in std.c
    // nasm -f win64 yourfile.asm -o yourfile.obj
    snprintf(nasm, sizeof(nasm), "nasm -f win64 \"%s\" -o hold.obj", output_file);
    nasm_result = system(nasm);

    // ERROR handling
    if (nasm_result != 0) {
        printf("ERROR - NASM (CRAW - ASM) failed with code %d\n", nasm_result);
        return;
    }


    char gcc[256];
    if (!strstr(exeFile, ".exe")) {
        snprintf(gcc, sizeof(gcc), "gcc -O2 hold.obj -o \"%s.exe\"", exeFile);
    } else {
        snprintf(gcc, sizeof(gcc), "gcc -O2 hold.obj -o \"%s\"", exeFile);
    }

    int gcc_result = system(gcc);

    if (gcc_result != 0) {
        printf("ERROR - GCC (ASM - BIN) failed with code %d\n", gcc_result);
        return;
    }

    delete_file("hold.obj");
    //period means it ran fine
    printf("...\n");
}
