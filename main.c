#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//include the prototype functions
#include "read/read.h"
#include "write/write.h"
#include "builder/build.h"
// not fully comforatble with c yet so I decided to write it in a lang i am more comfortable with
#include "std_compile/std.h"

//helper function to clear out whitespace and \n
void trim_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}


//what fiule is being acessed and its contenst
char main_file[100] = "input_output/file.craw";
char *fileContents;
// the file being out poutted to and what is being outputted
char output_file[100] = "input_output/main.asm";

//exe file final
char output_exe[100] = "a.exe";
char *outputContents;


int main(int argc, char* argv[]) {
    // Use command-line args if provided, otherwise prompt user
    
    if (argc >= 2) {
        strncpy(main_file, argv[1], sizeof(main_file));
        main_file[sizeof(main_file) - 1] = '\0'; // safety null terminate
    } else {
        printf("input file: ");
        fgets(main_file, sizeof(main_file), stdin);
        trim_newline(main_file);
    }

    if (argc >= 4 && strcmp(argv[2], "-o") == 0) {
        strncpy(output_exe, argv[3], sizeof(output_exe));
        output_exe[sizeof(output_exe) - 1] = '\0'; // safety null terminate
    } else {
        printf("output file: ");
        fgets(output_exe, sizeof(output_exe), stdin);
        trim_newline(output_exe);
    }

    //if input empty, set default filenames
    if (strlen(main_file) == 0) {
        strcpy(main_file, "input_output/file.craw");
    }
    if (strlen(output_exe) == 0) {
        strcpy(output_exe, "a.exe");
    }

    //call read file and then stoire the results
    fileContents = readFile();

    //check if there is anything in the file and return error
    if (fileContents == NULL) {
        printf("ERROR - File %s is empty or was failed to read", main_file);
        return 1;
    }

    size_t len = strlen(fileContents);
    while (len > 0 && (fileContents[len - 1] == '\0' || fileContents[len - 1] == '\r' || fileContents[len - 1] == '\n')) {
        fileContents[--len] = '\0';
    }

    // print the file contenst this can get annoying when it is compiling
    // printf("file contains: \n%s\n", fileContents);

    //comspile the file contents
    fileContents = std_compile(fileContents);

    //right to the file 
    printf("\nwriting to file %s\n", output_file);
    writeFile(fileContents);
    
    //dont let the memory leak like i did 
    free(fileContents);

    //build the exe
    build_exe_from_asm(output_exe);

    return 0;
}

