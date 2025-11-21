#include "preprocessor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// util - dynamically append characters/strings to a buffer
static void push_char(char **buf, size_t *len, size_t *cap, char c) {
    if (*len + 1 >= *cap) {
        *cap *= 2;
        *buf = realloc(*buf, *cap);
    }
    (*buf)[(*len)++] = c;
    (*buf)[*len] = '\0';
}

static void push_str(char **buf, size_t *len, size_t *cap, const char *s) {
    while (*s) {
        push_char(buf, len, cap, *s++);
    }
}

// removes comments of the input src
static char *remove_comments(const char *src) {
    size_t cap = strlen(src) * 2 + 32;
    size_t len = 0;
    char *out = malloc(cap);
    out[0] = '\0';

    const char *p = src;

    while (*p) {
        // // single line comment
        if (p[0] == '/' && p[1] == '/') {
            p += 2;
            while (*p && *p != '\n') p++;
            if (*p == '\n') {
                push_char(&out, &len, &cap, '\n');
                p++;
            }
        }
        // /* block comment */
        else if (p[0] == '/' && p[1] == '*') {
            p += 2;
            while (p[0] && !(p[0] == '*' && p[1] == '/')) {
                p++;
            }
            if (p[0] == '*' && p[1] == '/') p += 2;
        }
        // string literal: copy verbatim
        else if (*p == '"') {
            push_char(&out, &len, &cap, *p++);
            while (*p) {
                push_char(&out, &len, &cap, *p);
                if (*p == '\\' && p[1]) {
                    p++;
                    push_char(&out, &len, &cap, *p);
                } else if (*p == '"') {
                    p++;
                    break;
                }
                p++;
            }
        }
        // char literal: copy verbatim
        else if (*p == '\'') {
            push_char(&out, &len, &cap, *p++);
            while (*p) {
                push_char(&out, &len, &cap, *p);
                if (*p == '\\' && p[1]) {
                    p++;
                    push_char(&out, &len, &cap, *p);
                } else if (*p == '\'') {
                    p++;
                    break;
                }
                p++;
            }
        }
        else {
            push_char(&out, &len, &cap, *p++);
        }
    }

    return out;
}


// read file into string
static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *buf = malloc(size + 1);
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    return buf;
}

static char *process_includes(const char *src, int *err) {
    size_t cap = strlen(src) * 2 + 32;
    size_t len = 0;
    char *out = malloc(cap);
    out[0] = '\0';

    const char *p = src;

    while (*p) {
        // look for: use {filename};
        if (p[0] == 'u' && p[1] == 's' && p[2] == 'e' && isspace(p[3])) {
            p += 3;
            while (*p && isspace(*p)) p++;

            if (*p == '{') {
                p++;
                char filename[512];
                size_t fnlen = 0;

                while (*p && *p != '}') {
                    filename[fnlen++] = *p++;
                }
                filename[fnlen] = '\0';

                if (*p != '}') {
                    *err = 1;
                    return out;
                }
                p++;

                while (*p && isspace(*p)) p++;
                if (*p == ';') p++;

                char *file_contents = read_file(filename);
                if (!file_contents) {
                    *err = 1;
                    return out;
                }

                push_str(&out, &len, &cap, "{\n");
                push_str(&out, &len, &cap, file_contents);
                push_str(&out, &len, &cap, "\n}\n");
                free(file_contents);
                continue;
            }
        }

        push_char(&out, &len, &cap, *p++);
    }

    return out;
}

// escape output char
static const char *escape_char(char c) {
    switch (c) {
        case '\n': return "\\n";
        case '\t': return "\\t";
        case '\0': return "\\0";
        case '\\': return "\\\\";
        case '\'': return "\\'";
        default: {
            static char buf[4];
            buf[0] = c;
            buf[1] = '\0';
            return buf;
        }
    }
}


// Converts "hello" into {'h','e','l','l','o','\0'}
static char *process_string_literals(const char *src) {
    size_t cap = strlen(src) * 4 + 32;
    size_t len = 0;
    char *out = malloc(cap);
    out[0] = '\0';

    const char *p = src;

    while (*p) {
        if (*p == '"') {
            p++;

            push_char(&out, &len, &cap, '{');

            int first = 1;
            while (*p && *p != '"') {
                char ch;

                if (*p == '\\') {
                    p++;
                    switch (*p) {
                        case 'n': ch = '\n'; break;
                        case 't': ch = '\t'; break;
                        case '0': ch = '\0'; break;
                        case '\\': ch = '\\'; break;
                        case '\'': ch = '\''; break;
                        case '"': ch = '"'; break;
                        default: ch = *p; break;
                    }
                } else {
                    ch = *p;
                }

                if (!first) push_str(&out, &len, &cap, ", ");
                first = 0;

                push_char(&out, &len, &cap, '\'');
                push_str(&out, &len, &cap, escape_char(ch));
                push_char(&out, &len, &cap, '\'');

                p++;
            }

            // null terminator
            push_str(&out, &len, &cap, ", '\\0'}");

            if (*p == '"') p++;
        }
        else {
            push_char(&out, &len, &cap, *p++);
        }
    }

    return out;
}


static char *preprocess_once(const char *src, int *err) {
    char *no_comments = remove_comments(src);

    char *included = process_includes(no_comments, err);
    free(no_comments);
    if (*err) return included;

    char *strings = process_string_literals(included);
    free(included);

    return strings;
}

// repeats until stable text
char *preprocess(const char *source) {
    char *prev = NULL;
    char *cur = strdup(source);

    for (;;) {
        int err = 0;
        char *next = preprocess_once(cur, &err);

        if (prev && strcmp(prev, next) == 0) {
            free(prev);
            free(cur);
            return next;
        }

        free(prev);
        prev = next;

        free(cur);
        cur = strdup(prev);
    }
}

void free_preprocessed(char *s) {
    free(s);
}
