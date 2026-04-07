#include "setup.h"

static bool is_directory_exists(const char* path) {
    struct stat sb;

    // Use stat() to get file information. lstat() can be used to avoid
    // following symbolic links if needed.
    if (stat(path, &sb) == 0) {
        // check if the file type is a directory using the S_ISDIR macro
        return S_ISDIR(sb.st_mode);
    } else {
        /* 
        if stat() returns -1 (error), it may be due to:
        ENOENT: the directory does not exist.
        other errors (EACCES, ENOTDIR, etc.) mean it's not accessible as a directory.
        in all error cases, we return false.
        */
        return false;
    }
}

static int read_line(char *input) {
    int option;
    printf("%s", input);
    scanf("%d", &option);

    return option;
}

static void create_config_files(void) {
    write_file("~/.config/craw/which_assembler.bin", "1");
    write_file("~/.config/craw/version.txt", "0-1.0-GITHUB-RELEASE");
}


static void create_config_scratch(void) {
    int restart = read_line(
        "Creating config from scratch\n"
        "\n\tDo you want to start? [Y:1, N:0] : "
    );

    if (restart) { // new config
        create_config_files();
    }
    else {
        printf("Ending setup...");
        return;
    }
}


static void create_config(void) {
    int restart = read_line(
        "Config detected;\n"
        "\n\tDo you want to make a new config? [Y:1, N:0] : "
    );

    if (restart) { // new config
        create_config_files();
    }
    else {
        printf("Ending setup...");
        return;
    }
}

void setup(void) {
    // setup config
    if (is_directory_exists("~/.config/craw/")) {
        create_config();
    } 
    else {
        int new_config = read_line("Do you want to make a new config? [Y:1, N:0]");

        if (new_config) {
            create_config_scratch();
        } 
        else {
            printf("Ending setup...");
            return;
        }
    }
}
