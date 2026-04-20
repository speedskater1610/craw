#include "x86-64_codegen.h"
#include <stdio.h>
#include <stdbool.h>

#include "../codegen.h"
#include "../../parser/AST.h"
#include "../../lexer/token.h"

/* x86_64_StrBuf */
void x86_64_StrBuf_init(x86_64_StrBuf *sb) {
    sb->data = NULL;
    sb->len  = 0;
    sb->cap  = 0;
}

void x86_64_StrBuf_free(x86_64_StrBuf *sb) {
    free(sb->data);
    sb->data = NULL;
    sb->len  = 0;
    sb->cap  = 0;
}

void x86_64_StrBuf_append(x86_64_StrBuf *sb, const char *s) {
    size_t slen = strlen(s);
    if (sb->len + slen + 1 > sb->cap) {
        size_t new_cap = sb->cap ? sb->cap * 2 : 4096;
        while (new_cap < sb->len + slen + 1) new_cap *= 2;
        sb->data = realloc(sb->data, new_cap);
        sb->cap  = new_cap;
    }
    memcpy(sb->data + sb->len, s, slen + 1);
    sb->len += slen;
}

void x86_64_StrBuf_appendf(x86_64_StrBuf *sb, const char *fmt, ...) {
    char tmp[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);
    x86_64_StrBuf_append(sb, tmp);
}

/* Emit helpers - write into a given x86_64_StrBuf */
#define EMIT(buf, ...)   x86_64_StrBuf_appendf((buf), __VA_ARGS__)
#define EMITL(buf, ...)  do { x86_64_StrBuf_appendf((buf), __VA_ARGS__); x86_64_StrBuf_append((buf), "\n"); } while(0)

/* Current function body target */
#define B(cg)  (&(cg)->body)

/* Data section target */
#define D(cg)  (&(cg)->data)

/* Text section (for function labels etc.) */
#define T(cg)  (&(cg)->text)


/* Error */
static void cg_error(ELF32_Codegen *cg, const char *msg, const Token *tok) {
    if (tok)
        fprintf(stderr,
            "\e[31m\e[1m\e[4m\e[40mCODEGEN ERROR - \e[0m"
            " \e[4m\e[36m\e[40m%s\e[0m"
            "\n\t\e[4mAt line \e[0m\e[1m\e[33m\e[40m%u\e[0m"
            "\e[4m , column \e[0m\e[1m\e[33m\e[40m%u\e[0m\n",
            msg, tok->line, tok->column);
    else
        fprintf(stderr,
            "\e[31m\e[1m\e[4m\e[40mCODEGEN ERROR - \e[0m"
            " \e[4m\e[36m\e[40m%s\e[0m\n", msg);
    cg->had_error = true;
}


/* x86_64_SymTable */
static void x86_64_SymTable_reset(x86_64_SymTable *st) {
    st->count       = 0;
    st->next_offset = -4;
}

static int x86_64_SymTable_add_local(x86_64_SymTable *st, const char *name, int size) {
    if (st->count >= x86_64_Codegen_MAX_LOCALS) return 0;
    /* round size up to 4-byte slot */
    int slot = (size < 4) ? 4 : ((size + 3) & ~3);
    st->next_offset -= slot;   /* allocate from the bottom down */
    x86_64_LocalVar *v = &st->vars[st->count++];
    strncpy(v->name, name, sizeof(v->name) - 1);
    v->name[sizeof(v->name)-1] = '\0';
    v->offset       = st->next_offset;  /* base = bottom of allocated region */
    v->size         = slot;
    return v->offset;
}

static int x86_64_SymTable_add_param(x86_64_SymTable *st, const char *name, int param_index) {
    if (st->count >= x86_64_Codegen_MAX_LOCALS) return 8;
    x86_64_LocalVar *v = &st->vars[st->count++];
    strncpy(v->name, name, sizeof(v->name) - 1);
    v->name[sizeof(v->name)-1] = '\0';
    v->offset = 8 + param_index * 4;
    v->size   = 4;
    return v->offset;
}

static int x86_64_SymTable_find(const x86_64_SymTable *st, const char *name, bool *found) {
    for (int i = 0; i < st->count; i++) {
        if (strcmp(st->vars[i].name, name) == 0) {
            *found = true;
            return st->vars[i].offset;
        }
    }
    *found = false;
    return 0;
}

/* Total bytes needed for locals (always a multiple of 4). */
static int x86_64_SymTable_frame_size(const x86_64_SymTable *st) {
    int total = 0;
    for (int i = 0; i < st->count; i++)
        if (st->vars[i].offset < 0)
            total += st->vars[i].size;
    /* align to 16 bytes for ABI niceness */
    return (total + 15) & ~15;
}

/* Format an ebp-relative address correctly: [rbp - N] for negative, [rbp + N] for positive */
static void emit_rbp_ref(x86_64_StrBuf *buf, int offset) {
    if (offset < 0)
        x86_64_StrBuf_appendf(buf, "[rbp - %d]", -offset);
    else
        x86_64_StrBuf_appendf(buf, "[rbp + %d]", offset);
}

/* Type helpers */
static int type_size(const Token *tok) {
    if (!tok) return 4;
    switch (tok->tokenType) {
        case I16: case U16: return 2;
        case I64: case U64: return 8;
        case F64: return 8;
        case Char: return 1;
        default:   return 4;
    }
}

/* API */
x86_64_Codegen *x86_64_Codegen_new(void) {
    
}

void x86_64_Codegen_free(void) {

}

/* Compile a full program AST; fills cg->text and cg->data. */
void x86_64_Codegen_program() {

}