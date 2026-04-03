#ifndef SINGLE_TIME
#define SINGLE_TIME

// I am too lazy to write for loops so I wrote this
#define FOREACH(item_var, array) \
    for (int keep = 1, count = 0, size = sizeof(array) / sizeof(*(array)); \
         keep && count != size; \
         keep = !keep, count++) \
        for (item_var = (array) + count; keep; keep = !keep)

char *preprocess_funcExists(const char *src);

#endif