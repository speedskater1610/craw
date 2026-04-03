#define _POSIX_C_SOURCE 200809L 
// above: for strdup 
#include "preprocessor.h"
#include "func_exists.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// util - dynamically append characters/strings to a buffer;
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
        /*
         * Match two forms of use statement — both handled identically:
         *   use "filename";   (preferred, string-literal form)
         *   use {filename};   (legacy brace form)
         *
         * We must only match `use` when it appears as a whole word, not as a
         * substring of an identifier.  The character before `p` (if any) must
         * not be alphanumeric, and p[3] must be whitespace.
         */
        int at_word_start = (p == src) ||
                            !(isalnum((unsigned char)p[-1]) || p[-1] == '_');

        if (at_word_start &&
            p[0] == 'u' && p[1] == 's' && p[2] == 'e' && isspace((unsigned char)p[3])) {

            /* Peek ahead past whitespace to see what delimiter follows */
            const char *after_use = p + 3;
            while (*after_use && isspace((unsigned char)*after_use)) after_use++;

            char delim_open = 0, delim_close = 0;
            if (*after_use == '"') { delim_open = '"'; delim_close = '"'; }
            else if (*after_use == '{') { delim_open = '{'; delim_close = '}'; }

            if (delim_open) {
                after_use++; /* skip opening delimiter */
                char filename[512];
                size_t fnlen = 0;

                while (*after_use && *after_use != delim_close && fnlen < 511)
                    filename[fnlen++] = *after_use++;
                filename[fnlen] = '\0';

                if (*after_use != delim_close) { *err = 1; return out; }
                after_use++; /* skip closing delimiter */

                while (*after_use && isspace((unsigned char)*after_use)) after_use++;
                if (*after_use == ';') after_use++;

                char *file_contents = read_file(filename);
                if (!file_contents) {
                    fprintf(stderr, "use: cannot open '%s'\n", filename);
                    *err = 1;
                    return out;
                }

                /* Inline the file contents directly (no wrapping braces) */
                push_str(&out, &len, &cap, "\n");
                push_str(&out, &len, &cap, file_contents);
                push_str(&out, &len, &cap, "\n");
                free(file_contents);

                p = after_use;
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
    if (!source) return NULL;

    char *prev = NULL;
    char *next = NULL;
    char *cur = strdup(source);
    if (!cur) return NULL;

    for (;;) {
        int err = 0;
        next = preprocess_once(cur, &err);

        if (!next) {
            free(cur);
            free(prev);
            return NULL;
        }

        if (prev && strcmp(prev, next) == 0) {
            free(prev);
            free(cur);
            goto lblnext;
        }

        free(prev);
        prev = next;

        free(cur);
        cur = strdup(prev);
        if (!cur) {
            free(prev);
            return NULL;
        }
    }

lblnext:
    // one-time preprocessing: @funcExists<name> substitution
    {
        char *fe = preprocess_funcExists(next);
        if (fe) {
            free(next);
            next = fe;
        }
    }
    return next;
}


void free_preprocessed(char *s) {
    free(s);
}