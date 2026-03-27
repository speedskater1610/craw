#include "single_time.h"

static char **split_by_newline(char *str, int *count) {
    if (str == NULL)
        return NULL;

    if (count == NULL) 
        *count = 0;


    // count the number of lines first
    char *temp_str = strdup(str); // Work on a copy to count lines without modifying original
    if (temp_str == NULL)
        return NULL;
    
    char *token;
    int num_lines = 0;
    
    // Delimiters include newline (\n) and carriage return (\r) for cross-platform compatibility (more-modern russel here I am only supporting linux fools, I guess cross compiltion to linux too so this is neededededededededededededededededededededededdedededededededededededededededededdedededededededededededededed)
    for (   
            token = strtok(temp_str, "\n\r"); 
            token != NULL; 
            token = strtok(NULL, "\n\r")
        )
        num_lines++;

    free(temp_str); // Free the temporary copy

    // Alloc memory for the array of string pointers
    char **lines = (char **)malloc(sizeof(char *) * (num_lines + 1)); // +1 for a NULL sentinel
    if (lines == NULL) {
        return NULL;
    }

    // split the original string and store the pointers
    int i = 0;
    // NOTE: strtok modifies the original string by replacing delimiters with null terminators
    for (
            token = strtok(str, "\n\r"); 
            token != NULL; 
            token = strtok(NULL, "\n\r")
        ) {
        // Dynamically allocate memory for each line and copy the token
        lines[i] = (char *)malloc(strlen(token) + 1);
        if (lines[i] == NULL) {
            // cleanup in case of allocation failure
            for (int j = 0; j < i; j++) {
                free(lines[j]);
            }
            free(lines);
            return NULL;
        }
        strcpy(lines[i], token);
        i++;
    }
    lines[i] = NULL; // Add a NULL sentinel at the end
    *count = num_lines;
    return lines;

    // Why tf is it called a sentinel for arrays but terminator for char arrays
    // maybe because \0 is a char for NULL but the sentinel is just NULL?
}

static void free_split_array(char **array, int size) {
    if (array == NULL) {
        return;
    }
    for (int i = 0; i < size; i++) {
        free(array[i]); // Free each individual string
    }
    free(array); // Free the array of pointers itself
}

static char *foundAt(char *src, int indexAt) {
    // if index at + the len of "funcExists" then find the fn name between <> removing whitespace and then returning that

    return ""; // TODO: rm
}

char *preprocess_funcExists(const char *src) {
    for (;;) {
        int count = 0;
        char **split_src = split_by_newline(src, &count);
        char **item;
        char *findFunc = "\0";

        FOREACH(item, split_src) {
            int len = strlen(*item);
            
            for(int i = 0; i < len;  i++) {
                if (*item[i] == '@') 
                    findFunc = foundAt(*item, i);
                // once the name of the fn is found then search the entire src code for  
            }
        }

        return ""; // TODO: return the entire soruce but all of the @funcExists<...>'s replaced with a 0 or 1 for booleans 
    }
}