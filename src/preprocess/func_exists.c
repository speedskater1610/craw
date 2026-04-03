#define _POSIX_C_SOURCE 200809L
#include "func_exists.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * preprocess_funcExists
 *
 * Replaces every occurrence of  @funcExists<name>  in `src` with
 * "1" if a function named `name` is defined anywhere in `src`,
 * or "0" otherwise.
 *
 * A function definition is detected by the presence of:
 *   fn <name>
 * anywhere in the source text (after any earlier preprocessing passes).
 *
 * Returns a newly heap-allocated string.  Caller must free().
 * Returns NULL on allocation failure.
 */


/* Helper: return 1 if `fn_name` appears as a function name in src. */
/* We look for the pattern "fn " followed by exactly fn_name followed */
/* by a non-identifier character. */

static int function_defined(const char *src, const char *fn_name) {
    size_t name_len = strlen(fn_name);
    const char *p = src;
    while ((p = strstr(p, "fn ")) != NULL) {
        p += 3; /* skip "fn " */
        /* skip any extra spaces */
        while (*p == ' ' || *p == '\t') p++;
        if (strncmp(p, fn_name, name_len) == 0) {
            /* make sure it's a word boundary (not a prefix) */
            char after = p[name_len];
            if (after == '\0' || after == ' ' || after == '\t' ||
                after == '\n' || after == '\r' || after == '(') {
                return 1;
            }
        }
    }
    return 0;
}


/* Dynamic string builder */

typedef struct { char *buf; size_t len; size_t cap; } SB;

static int sb_push(SB *sb, const char *s, size_t n) {
    if (sb->len + n + 1 > sb->cap) {
        size_t nc = sb->cap ? sb->cap * 2 : 256;
        while (nc < sb->len + n + 1) nc *= 2;
        char *nb = realloc(sb->buf, nc);
        if (!nb) return 0;
        sb->buf = nb; sb->cap = nc;
    }
    memcpy(sb->buf + sb->len, s, n);
    sb->len += n;
    sb->buf[sb->len] = '\0';
    return 1;
}

static int sb_pushc(SB *sb, char c) { return sb_push(sb, &c, 1); }


/* Main replacement pass */

char *preprocess_funcExists(const char *src) {
    if (!src) return NULL;

    SB out = {0};
    const char *p = src;

    while (*p) {
        /* Look for @funcExists< */
        if (p[0] == '@' && strncmp(p, "@funcExists<", 12) == 0) {
            const char *name_start = p + 12;
            const char *name_end   = strchr(name_start, '>');
            if (name_end) {
                /* Extract function name */
                size_t name_len = (size_t)(name_end - name_start);
                char *fn_name = malloc(name_len + 1);
                if (!fn_name) { free(out.buf); return NULL; }
                memcpy(fn_name, name_start, name_len);
                fn_name[name_len] = '\0';

                /* Trim whitespace from name */
                char *ns = fn_name;
                while (*ns == ' ' || *ns == '\t') ns++;
                char *ne = fn_name + name_len - 1;
                while (ne >= ns && (*ne == ' ' || *ne == '\t')) ne--;
                ne[1] = '\0';

                /* Emit "1" or "0" */
                const char *rep = function_defined(src, ns) ? "1" : "0";
                if (!sb_push(&out, rep, 1)) { free(fn_name); free(out.buf); return NULL; }

                free(fn_name);
                p = name_end + 1; /* skip past '>' */
                continue;
            }
        }
        if (!sb_pushc(&out, *p)) { free(out.buf); return NULL; }
        p++;
    }

    if (!out.buf) {
        /* src was empty — return empty string */
        char *r = malloc(1);
        if (r) r[0] = '\0';
        return r;
    }
    return out.buf;
}