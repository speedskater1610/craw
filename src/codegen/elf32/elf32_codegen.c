#define _POSIX_C_SOURCE 200809L
/*
 * x86-32 CRASM code generator for CRAW.
 *
 * Two-buffer strategy per function
 * ---------------------------------
 * We write the function body into cg->body and count locals as we go.
 * When the function ends we know the exact local-frame size and can emit
 * the correct  sub esp, N  in the prologue before splicing body in.
 *
 * String literals
 * ---------------
 * Every string literal is interned into cg->strings[].  After all
 * functions are compiled, ELF32_Codegen_get_asm() prepends a data section
 * that pushes the bytes on the stack at program start — actually we
 * emit them as labelled byte sequences using the "push imm" trick since
 * CRASM doesn't support .data.  Strings are built backwards on the stack
 * by the _data_init helper and their stack address is patched into eax
 * wherever they are used.
 *
 * CRASM limitations worked around
 * --------------------------------
 *  - No .data / db directive -> strings pushed byte-by-byte at runtime
 *  - shl/shr only take imm8 -> shift count must be immediate
 *  - No cdq instruction -> use explicit sign extend via sar
 *  - idiv needs edx:eax -> we always zero/sign-extend before div
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#include "../codegen.h"
#include "../../parser/AST.h"
#include "../../lexer/token.h"


/* ELF32_StrBuf */
void ELF32_StrBuf_init(ELF32_StrBuf *sb) {
    sb->data = NULL;
    sb->len  = 0;
    sb->cap  = 0;
}

void ELF32_StrBuf_free(ELF32_StrBuf *sb) {
    free(sb->data);
    sb->data = NULL;
    sb->len  = 0;
    sb->cap  = 0;
}

void ELF32_StrBuf_append(ELF32_StrBuf *sb, const char *s) {
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

void ELF32_StrBuf_appendf(ELF32_StrBuf *sb, const char *fmt, ...) {
    char tmp[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);
    ELF32_StrBuf_append(sb, tmp);
}

 
/* Emit helpers - write into a given ELF32_StrBuf */
#define EMIT(buf, ...)   ELF32_StrBuf_appendf((buf), __VA_ARGS__)
#define EMITL(buf, ...)  do { ELF32_StrBuf_appendf((buf), __VA_ARGS__); ELF32_StrBuf_append((buf), "\n"); } while(0)

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


/* ELF32_SymTable */
static void ELF32_SymTable_reset(ELF32_SymTable *st) {
    st->count       = 0;
    st->next_offset = -4;
}

static int ELF32_SymTable_add_local(ELF32_SymTable *st, const char *name, int size) {
    if (st->count >= ELF32_Codegen_MAX_LOCALS) return 0;
    /* round size up to 4-byte slot */
    int slot = (size < 4) ? 4 : ((size + 3) & ~3);
    st->next_offset -= slot;   /* allocate from the bottom down */
    ELF32_LocalVar *v = &st->vars[st->count++];
    strncpy(v->name, name, sizeof(v->name) - 1);
    v->name[sizeof(v->name)-1] = '\0';
    v->offset       = st->next_offset;  /* base = bottom of allocated region */
    v->size         = slot;
    return v->offset;
}

static int ELF32_SymTable_add_param(ELF32_SymTable *st, const char *name, int param_index) {
    if (st->count >= ELF32_Codegen_MAX_LOCALS) return 8;
    ELF32_LocalVar *v = &st->vars[st->count++];
    strncpy(v->name, name, sizeof(v->name) - 1);
    v->name[sizeof(v->name)-1] = '\0';
    v->offset = 8 + param_index * 4;
    v->size   = 4;
    return v->offset;
}

stati int ELF32_SymTable_find(const ELF32_SymTable *st, const char *name, bool *found) {
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
static int ELF32_SymTable_frame_size(const ELF32_SymTable *st) {
    int total = 0;
    for (int i = 0; i < st->count; i++)
        if (st->vars[i].offset < 0)
            total += st->vars[i].size;
    /* align to 16 bytes for ABI niceness */
    return (total + 15) & ~15;
}

/* Format an ebp-relative address correctly: [ebp - N] for negative, [ebp + N] for positive */
static void emit_ebp_ref(ELF32_StrBuf *buf, int offset) {
    if (offset < 0)
        ELF32_StrBuf_appendf(buf, "[ebp - %d]", -offset);
    else
        ELF32_StrBuf_appendf(buf, "[ebp + %d]", offset);
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

/* Struct type table */
 

/* Register a struct definition from a NODE_DEF_STRUCT node.
 * NODE_LET children: [0]=name, [1]=type(defstruct), [2]=NODE_DEF_STRUCT
 * NODE_DEF_STRUCT children: NODE_STRUCT_FIELD nodes, each with:
 *   token = field name, children[0] = NODE_TYPE
 */
static void register_struct(ELF32_Codegen *cg, const Ast_node *let_node) {
    if (let_node->children.size < 3) return;
    const char *sname = let_node->children.items[0]->token
                        ? let_node->children.items[0]->token->lexeme : "";
    const Ast_node *ds = let_node->children.items[2];

    if (cg->structs.count >= ELF32_Codegen_MAX_STRUCTS) return;
    ELF32_StructDef *sd = &cg->structs.defs[cg->structs.count++];
    strncpy(sd->name, sname, sizeof(sd->name)-1);
    sd->nfields    = 0;
    sd->total_size = 0;

    for (size_t i = 0; i < ds->children.size && sd->nfields < ELF32_Codegen_MAX_STRUCT_FIELDS; i++) {
        const Ast_node *field = ds->children.items[i];
        if (field->kind != NODE_STRUCT_FIELD) continue;

        const char *fname = field->token ? field->token->lexeme : "";
        const Token *ftype = (field->children.size > 0) ? field->children.items[0]->token : NULL;
        int fsz = type_size(ftype);
        /* Align field to its size (max 4) */
        int align = fsz < 4 ? fsz : 4;
        int padded = (sd->total_size + align - 1) & ~(align - 1);

        ELF32_StructField *sf = &sd->fields[sd->nfields++];
        strncpy(sf->name, fname, sizeof(sf->name)-1);
        sf->offset = padded;
        sf->size   = fsz;
        sd->total_size = padded + fsz;
    }
    /* Round total size up to 4-byte boundary */
    sd->total_size = (sd->total_size + 3) & ~3;
}

static const ELF32_StructDef *find_struct(const ELF32_Codegen *cg, const char *name) {
    for (int i = 0; i < cg->structs.count; i++)
        if (strcmp(cg->structs.defs[i].name, name) == 0)
            return &cg->structs.defs[i];
    return NULL;
}

static __attribute__((unused)) const ELF32_StructField *find_field(const ELF32_StructDef *sd, const char *name) {
    for (int i = 0; i < sd->nfields; i++)
        if (strcmp(sd->fields[i].name, name) == 0)
            return &sd->fields[i];
    return NULL;
}

/* Find which struct type a variable was declared as.
   We store the struct name in the ELF32_LocalVar name as "varname:StructName". */
static __attribute__((unused)) const ELF32_StructDef *var_struct_type(const ELF32_Codegen *cg, const char *varname) {
    /* Look for "varname:StructName" pattern in symbol table */
    for (int i = 0; i < cg->syms.count; i++) {
        const char *vn = cg->syms.vars[i].name;
        const char *colon = strchr(vn, ':');
        if (colon) {
            size_t nlen = (size_t)(colon - vn);
            if (strncmp(vn, varname, nlen) == 0 && vn[nlen] == ':')
                return find_struct(cg, colon + 1);
        }
    }
    return NULL;
}

 
 
/* String literal interning */
/* Process escape sequences in a raw lexer string and intern it.
   Returns the asm label for this string. */
static const char *intern_string(ELF32_Codegen *cg, const char *raw) {
    /* Check if already interned */
    for (int i = 0; i < cg->nstrings; i++)
        if (strcmp(cg->strings[i].value, raw) == 0)
            return cg->strings[i].label;

    if (cg->nstrings >= ELF32_Codegen_MAX_STRINGS) return ".str_overflow";

    ELF32_StrEntry *e = &cg->strings[cg->nstrings++];
    snprintf(e->label, sizeof(e->label), ".str%d", cg->nstrings - 1);
    e->value = strdup(raw);
    e->len   = (int)strlen(raw) + 1; /* include null terminator */
    return e->label;
}

 
/* Public API */
 

ELF32_Codegen *ELF32_Codegen_new(void) {
    ELF32_Codegen *cg = calloc(1, sizeof(ELF32_Codegen));
    ELF32_StrBuf_init(&cg->text);
    ELF32_StrBuf_init(&cg->data);
    ELF32_StrBuf_init(&cg->pre);
    ELF32_StrBuf_init(&cg->body);
    cg->label_counter = 0;
    cg->had_error     = false;
    cg->nglobals      = 0;
    ELF32_SymTable_reset(&cg->syms);
    return cg;
}

void ELF32_Codegen_free(ELF32_Codegen *cg) {
    ELF32_StrBuf_free(&cg->text);
    ELF32_StrBuf_free(&cg->data);
    ELF32_StrBuf_free(&cg->pre);
    ELF32_StrBuf_free(&cg->body);
    for (int i = 0; i < cg->nstrings; i++)
        free(cg->strings[i].value);
    free(cg);
}

void ELF32_Codegen_fresh_label(ELF32_Codegen *cg, char *buf, const char *prefix) {
    snprintf(buf, 32, ".%s%d", prefix, cg->label_counter++);
}

/* Build the string-data init function that pushes string bytes onto
   the stack and saves each pointer into a per-string slot.
   We use a simple scheme: for each interned string we push its bytes
   in reverse order (null first), then mov the stack pointer into a
   reserved local in _data_init's frame.  The pointer lives at a fixed
   negative offset from the _data_init frame's ebp and is accessed via
   a global label trick however CRASM has no globals, so instead we
   store each string address in a caller-controlled memory slot.

   Simpler approach used here: push bytes of every string at _start,
   record esp as the base pointer right after, and store it in a
   dedicated slot.  Each string occupies (len) bytes on the stack with
   the null terminator pushed first (so stack grows downward but string
   is laid out correctly after the first push).

   We emit a helper _craw_str_init that sets up all strings and stores
   their addresses into statically-known stack slots in _start's frame.
*/
static void emit_string_data(ELF32_Codegen *cg) {
    if (cg->nstrings == 0) return;

    EMITL(D(cg), "; === string literal helper ===");
    EMITL(D(cg), "_craw_str_init:");
    EMITL(D(cg), "    push ebp");
    EMITL(D(cg), "    mov ebp, esp");

    /* For each string, push its bytes in REVERSE order so that when
       read from low address upward they form the correct string.
       Stack grows downward, so push last byte first. */
    for (int si = 0; si < cg->nstrings; si++) {
        const char *s = cg->strings[si].value;
        int len = (int)strlen(s);

        EMITL(D(cg), "    ; string %s = \"%s\"", cg->strings[si].label, s);
        /* push null terminator first (it ends up at highest address = end of string) */
        EMITL(D(cg), "    push 0");
        /* push bytes in reverse so str[0] is at lowest address */
        for (int bi = len - 1; bi >= 0; bi--) {
            unsigned char c = (unsigned char)s[bi];
            EMITL(D(cg), "    push %u", (unsigned)c);
        }
        /* esp now points to the start of the string */
        EMITL(D(cg), "    ; esp = &%s", cg->strings[si].label);
        /* Store address into param slot so caller can retrieve it.
           We pass a pointer-array base in eax. */
        EMITL(D(cg), "    mov [eax + %d], esp", si * 4);
    }

    EMITL(D(cg), "    pop ebp");
    EMITL(D(cg), "    ret");
    EMITL(D(cg), "");
}

char *ELF32_Codegen_get_asm(ELF32_Codegen *cg) {
    /* Build data section helpers first */
    emit_string_data(cg);

    /* Concatenate: data helpers + text */
    size_t total = (cg->data.len + cg->text.len + 16);
    char *out = malloc(total);
    out[0] = '\0';
    if (cg->data.len) strcat(out, cg->data.data);
    if (cg->text.len) strcat(out, cg->text.data);
    return out;
}

void ELF32_Codegen_write_asm(ELF32_Codegen *cg, FILE *f) {
    char *asm_str = ELF32_Codegen_get_asm(cg);
    fputs(asm_str, f);
    free(asm_str);
}

 
/* Forward declarations */
 

static void cg_stmt  (ELF32_Codegen *cg, const Ast_node *node);
static void cg_expr  (ELF32_Codegen *cg, const Ast_node *node);
static void cg_fn_def(ELF32_Codegen *cg, const Ast_node *node);
static void cg_let   (ELF32_Codegen *cg, const Ast_node *node);
static void cg_return(ELF32_Codegen *cg, const Ast_node *node);
static void cg_if    (ELF32_Codegen *cg, const Ast_node *node);
static void cg_while (ELF32_Codegen *cg, const Ast_node *node);
static void cg_goto  (ELF32_Codegen *cg, const Ast_node *node);
static void cg_label (ELF32_Codegen *cg, const Ast_node *node);
static void cg_block (ELF32_Codegen *cg, const Ast_node *node);
static void cg_call  (ELF32_Codegen *cg, const Ast_node *node);
static void cg_binary(ELF32_Codegen *cg, const Ast_node *node);
static void cg_unary (ELF32_Codegen *cg, const Ast_node *node);
static void cg_assign(ELF32_Codegen *cg, const Ast_node *node);

 
/* Program */
 

 
/* Global variable support */
/* */
/* Globals are stored on _start's stack frame (which is never freed */
/* since _start calls sys_exit, not ret).  We reserve space for each */
/* global below _start's initial esp, then store the base pointer in */
/* esi so all functions can access globals as [esi + offset]. */
/* */
/* Register convention (craw-specific extension to cdecl): */
/* esi = global variable base pointer (set once in _start, read-only) */
 

/* Look up a global by name; returns its esi-relative offset or -1 */
static int global_find(ELF32_Codegen *cg, const char *name) {
    int offset = 0;
    for (int i = 0; i < cg->nglobals; i++) {
        if (strcmp(cg->globals[i].name, name) == 0)
            return offset;
        offset += cg->globals[i].size < 4 ? 4 : cg->globals[i].size;
    }
    return -1;
}

/* Total bytes needed for all globals */
static int globals_frame_size(ELF32_Codegen *cg) {
    int total = 0;
    for (int i = 0; i < cg->nglobals; i++)
        total += cg->globals[i].size < 4 ? 4 : cg->globals[i].size;
    return (total + 15) & ~15;  /* align to 16 */
}

void ELF32_Codegen_program(ELF32_Codegen *cg, const Ast_node *program) {
    EMITL(T(cg), "; === CRAW generated x86-32 assembly ===");

    /* First pass: register struct types and collect global variables */
    for (size_t i = 0; i < program->children.size; i++) {
        const Ast_node *child = program->children.items[i];
        if (child->kind != NODE_LET || child->children.size < 2) continue;
        const Token *type_tok = child->children.items[1]->token;
        if (type_tok && type_tok->tokenType == DefStruct) {
            /* Register struct layout globally so all functions see it */
            register_struct(cg, child);
        } else {
            /* Non-struct global variable */
            const char *vname = child->children.items[0]->token
                                ? child->children.items[0]->token->lexeme : "";
            int sz = type_size(type_tok);
            if (cg->nglobals < ELF32_Codegen_MAX_GLOBALS) {
                snprintf(cg->globals[cg->nglobals].label, 32,
                         ".gbl%d", cg->nglobals);
                strncpy(cg->globals[cg->nglobals].name, vname, 63);
                cg->globals[cg->nglobals].size = sz;
                cg->nglobals++;
            }
        }
    }

    /* _start: set up global frame, init globals, then call main */
    EMITL(T(cg), "_start:");

    int gframe = globals_frame_size(cg);
    if (gframe > 0) {
        EMITL(T(cg), "    sub esp, %d    ; reserve global variable storage", gframe);
        EMITL(T(cg), "    mov esi, esp   ; esi = global base pointer");
    }

    EMITL(T(cg), "    call main");
    EMITL(T(cg), "    mov ebx, eax     ; exit code = return of main");
    EMITL(T(cg), "    mov eax, 1       ; sys_exit");
    EMITL(T(cg), "    int 0x80");
    EMITL(T(cg), "");

    /* Compile functions, and emit global initialisers as a special section */
    for (size_t i = 0; i < program->children.size; i++) {
        const Ast_node *child = program->children.items[i];
        if (child->kind == NODE_FN_DEF)
            cg_fn_def(cg, child);
        else if (child->kind == NODE_LET && child->children.size >= 3) {
            /* Top-level let with initialiser — we need a _craw_init function */
            /* For now emit a comment; full init function emitted below */
        }
    }

    /* Emit global init function if any globals have initialisers */
    int has_init = 0;
    for (size_t i = 0; i < program->children.size; i++) {
        if (program->children.items[i]->kind == NODE_LET &&
            program->children.items[i]->children.size >= 3)
            has_init = 1;
    }
    (void)has_init; /* will use in future when we add _craw_init call to _start */
}

 
/* Function definition */

/*
 * fn_def children:
 *   [0]         NODE_IDENTIFIER  function name
 *   [1]         NODE_TYPE        return type
 *   [2..n-2]    NODE_PARAM       parameters
 *   [n-1]       NODE_PROGRAM     body block
 */
static void cg_fn_def(ELF32_Codegen *cg, const Ast_node *node) {
    if (node->children.size < 2) {
        cg_error(cg, "Malformed fn_def", node->token);
        return;
    }

    const char *fname = node->children.items[0]->token
                        ? node->children.items[0]->token->lexeme : "unknown";
    strncpy(cg->cur_fn, fname, sizeof(cg->cur_fn) - 1);
    cg->cur_fn[sizeof(cg->cur_fn)-1] = '\0';

    ELF32_SymTable_reset(&cg->syms);

    /* Collect params and find body */
    int param_index = 0;
    const Ast_node *body = NULL;
    for (size_t i = 2; i < node->children.size; i++) {
        const Ast_node *ch = node->children.items[i];
        if (ch->kind == NODE_PARAM) {
            const char *pname = ch->token ? ch->token->lexeme : "";
            ELF32_SymTable_add_param(&cg->syms, pname, param_index++);
        } else if (ch->kind == NODE_PROGRAM) {
            body = ch;
        }
    }
    if (!body && node->children.size > 0)
        body = node->children.items[node->children.size - 1];

    /* Reset body buffer for this function */
    ELF32_StrBuf_free(&cg->body);
    ELF32_StrBuf_init(&cg->body);

    /* Compile body into cg->body */
    if (body) cg_block(cg, body);

    /* Now we know the frame size */
    int frame_sz = ELF32_SymTable_frame_size(&cg->syms);
    /* Minimum 16 bytes even for empty functions */
    if (frame_sz < 16) frame_sz = 16;

    /* Emit into text section */
    int has_globals = (cg->nglobals > 0);
    EMITL(T(cg), "");
    EMITL(T(cg), "; --- fn %s ---", fname);
    EMITL(T(cg), "%s:", fname);
    EMITL(T(cg), "    push ebp");
    EMITL(T(cg), "    mov ebp, esp");
    /* Save esi (global base pointer) — it is callee-saved in cdecl */
    if (has_globals)
        EMITL(T(cg), "    push esi");
    if (frame_sz > 0)
        EMITL(T(cg), "    sub esp, %d", frame_sz);

    /* Splice in body */
    if (cg->body.len)
        ELF32_StrBuf_append(T(cg), cg->body.data);

    /* Epilogue */
    EMITL(T(cg), ".%s_exit:", fname);
    /* Restore esi before cleaning up frame */
    if (has_globals) {
        EMITL(T(cg), "    lea esp, [ebp - 4]   ; point esp at saved esi slot");
        EMITL(T(cg), "    pop esi");
    }
    EMITL(T(cg), "    mov esp, ebp");
    EMITL(T(cg), "    pop ebp");
    EMITL(T(cg), "    ret");
}

  
/* Block */
static void cg_block(ELF32_Codegen *cg, const Ast_node *node) {
    for (size_t i = 0; i < node->children.size; i++)
        cg_stmt(cg, node->children.items[i]);
}

 
/* Statements */
static void cg_stmt(ELF32_Codegen *cg, const Ast_node *node) {
    switch (node->kind) {
        case NODE_LET:       cg_let(cg, node);    break;
        case NODE_RETURN:    cg_return(cg, node);  break;
        case NODE_IF:        cg_if(cg, node);      break;
        case NODE_WHILE:     cg_while(cg, node);   break;
        case NODE_GOTO:      cg_goto(cg, node);    break;
        case NODE_LABEL:     cg_label(cg, node);   break;
        case NODE_PROGRAM:   cg_block(cg, node);   break;
        case NODE_EXPR_STMT:
            if (node->children.size > 0)
                cg_expr(cg, node->children.items[0]);
            break;
        case NODE_ASM_BLOCK: {
            /* Emit assembly tokens preserving their original line structure.
               Each token carries a line number from the lexer. When the line
               number changes we emit a newline so the assembler sees one
               instruction per line. Labels (IDENT followed by COLON) are
               emitted without leading spaces. */
            EMITL(B(cg), "    ; --- inline asm ---");
            unsigned int cur_line = 0;
            for (size_t i = 0; i < node->children.size; i++) {
                const Token *t = node->children.items[i]->token;
                if (!t || !t->lexeme) continue;

                /* New source line → start a new assembly line.
                   If the NEXT token is a colon this is a label — no indent. */
                if (t->line != cur_line) {
                    if (cur_line != 0) ELF32_StrBuf_append(B(cg), "\n");
                    /* Peek ahead: is the token after this one a Colon? */
                    bool is_label = false;
                    if (i + 1 < node->children.size) {
                        const Token *next_t = node->children.items[i+1]->token;
                        if (next_t && next_t->tokenType == Colon) is_label = true;
                    }
                    ELF32_StrBuf_append(B(cg), is_label ? "" : "    ");
                    cur_line = t->line;
                }

                /* Colon = label suffix — trim any trailing space then newline.
                   Labels must be at the start of the line with no indent. */
                if (t->tokenType == Colon) {
                    /* Remove trailing space from the identifier just emitted */
                    ELF32_StrBuf *b = B(cg);
                    if (b->len > 0 && b->data[b->len-1] == ' ') {
                        b->len--;
                        b->data[b->len] = '\0';
                    }
                    ELF32_StrBuf_append(B(cg), ":\n");
                    cur_line = 0;
                    continue;
                }

                /* Brackets: no spaces inside [reg+disp] */
                if (t->tokenType == LeftBracket) {
                    ELF32_StrBuf_append(B(cg), "[");
                    continue;
                }
                if (t->tokenType == RightBracket) {
                    ELF32_StrBuf_append(B(cg), "]");
                    continue;
                }

                /* Comma: no space before */
                if (t->tokenType == Comma) {
                    ELF32_StrBuf_append(B(cg), ", ");
                    continue;
                }

                EMIT(B(cg), "%s ", t->lexeme);
            }
            ELF32_StrBuf_append(B(cg), "\n");
            EMITL(B(cg), "    ; --- end inline asm ---");
            break;
        }
        case NODE_DEF_STRUCT:
            EMITL(B(cg), "    ; struct definition (compile-time only)");
            break;
        default:
            break;
    }
}

 
/* let */

static void cg_let(ELF32_Codegen *cg, const Ast_node *node) {
    if (node->children.size < 2) {
        cg_error(cg, "Malformed let", node->token);
        return;
    }
    const char *vname = node->children.items[0]->token
                        ? node->children.items[0]->token->lexeme : "";
    const Token *type_tok = node->children.items[1]->token;

    /* defstruct definition: register layout, allocate no space for the type itself */
    if (type_tok && type_tok->tokenType == DefStruct) {
        if (node->children.size > 2)
            register_struct(cg, node);
        EMITL(B(cg), "    ; struct %s defined", vname);
        return;
    }

    /* structinstance: allocate struct body on stack */
    if (type_tok && type_tok->tokenType == StructInstance) {
        /* The initialiser should be NODE_STRUCT_NEW with the struct type name */
        const char *sname = NULL;
        if (node->children.size > 2) {
            const Ast_node *init = node->children.items[2];
            if (init->kind == NODE_STRUCT_NEW && init->children.size > 0)
                sname = init->children.items[0]->token
                        ? init->children.items[0]->token->lexeme : NULL;
        }
        const ELF32_StructDef *sd = sname ? find_struct(cg, sname) : NULL;
        int sz = sd ? sd->total_size : 4;

        /* Store as "varname:StructName" so we can look up the type later */
        char tagged[128];
        snprintf(tagged, sizeof(tagged), "%s:%s", vname, sname ? sname : "?");
        int offset = ELF32_SymTable_add_local(&cg->syms, tagged, sz);

        EMITL(B(cg), "    ; let %s : %s  [ebp%d]  size=%d",
              vname, sname ? sname : "?", offset, sz);

        /* Zero-initialise the struct on the stack */
        for (int i = 0; i < sz; i += 4) {
            EMITL(B(cg), "    mov eax, 0");
            EMIT(B(cg), "    mov "); emit_ebp_ref(B(cg), offset + i);
            ELF32_StrBuf_append(B(cg), ", eax\n");
        }
        return;
    }

    /* Ordinary variable */
    int sz = type_size(type_tok);
    int offset = ELF32_SymTable_add_local(&cg->syms, vname, sz);
    EMITL(B(cg), "    ; let %s : %s  [ebp%d]",
          vname, type_tok ? type_tok->lexeme : "?", offset);

    if (node->children.size > 2) {
        cg_expr(cg, node->children.items[2]);
        EMIT(B(cg), "    mov "); emit_ebp_ref(B(cg), offset); ELF32_StrBuf_appendf(B(cg), ", eax    ; store %s\n", vname);
    }
}

 
/* return */
 

static void cg_return(ELF32_Codegen *cg, const Ast_node *node) {
    if (node->children.size > 0)
        cg_expr(cg, node->children.items[0]);
    else
        EMITL(B(cg), "    xor eax, eax");
    EMITL(B(cg), "    jmp .%s_exit", cg->cur_fn);
}

 
/* if */
 


/* Returns true if the last statement in a block is a return or goto
   (meaning any jmp after it would be dead code). */
static bool block_ends_with_transfer(const Ast_node *block) {
    if (!block || block->children.size == 0) return false;
    const Ast_node *last = block->children.items[block->children.size - 1];
    return (last->kind == NODE_RETURN || last->kind == NODE_GOTO);
}

/*
 * if children:
 *   [0]   condition expr
 *   [1]   then-block (NODE_PROGRAM)
 *   [2]?  else-block (NODE_PROGRAM) or else-if (NODE_IF)
 */
static void cg_if(ELF32_Codegen *cg, const Ast_node *node) {
    if (node->children.size < 2) {
        cg_error(cg, "Malformed if", node->token);
        return;
    }
    bool has_else = (node->children.size >= 3);

    char else_lbl[32], end_lbl[32];
    ELF32_Codegen_fresh_label(cg, end_lbl, "if_end");
    if (has_else) ELF32_Codegen_fresh_label(cg, else_lbl, "if_else");

    /* Evaluate condition */
    cg_expr(cg, node->children.items[0]);
    EMITL(B(cg), "    test eax, eax");
    EMITL(B(cg), "    je %s", has_else ? else_lbl : end_lbl);

    /* Then branch */
    cg_block(cg, node->children.items[1]);

    if (has_else) {
        if (!block_ends_with_transfer(node->children.items[1]))
            EMITL(B(cg), "    jmp %s", end_lbl);
        EMITL(B(cg), "%s:", else_lbl);
        /* else child can be another if (else-if) or a plain block */
        const Ast_node *else_node = node->children.items[2];
        if (else_node->kind == NODE_IF) {
            cg_if(cg, else_node);
        } else {
            cg_block(cg, else_node);
        }
    }

    EMITL(B(cg), "%s:", end_lbl);
}

/*
 * while children:
 *   [0]  condition expr
 *   [1]  body block
 */
static void cg_while(ELF32_Codegen *cg, const Ast_node *node) {
    if (node->children.size < 2) {
        cg_error(cg, "Malformed while", node->token);
        return;
    }
    char top_lbl[32], end_lbl[32];
    ELF32_Codegen_fresh_label(cg, top_lbl, "while_top");
    ELF32_Codegen_fresh_label(cg, end_lbl, "while_end");

    EMITL(B(cg), "%s:", top_lbl);
    cg_expr(cg, node->children.items[0]);
    EMITL(B(cg), "    test eax, eax");
    EMITL(B(cg), "    je %s", end_lbl);
    cg_block(cg, node->children.items[1]);
    EMITL(B(cg), "    jmp %s", top_lbl);
    EMITL(B(cg), "%s:", end_lbl);
}

 
/* goto / label */
 

static void cg_goto(ELF32_Codegen *cg, const Ast_node *node) {
    if (!node->children.size) return;
    const char *lbl = node->children.items[0]->token
                      ? node->children.items[0]->token->lexeme : "";
    EMITL(B(cg), "    jmp %s", lbl);
}

static void cg_label(ELF32_Codegen *cg, const Ast_node *node) {
    if (!node->children.size) return;
    const char *lbl = node->children.items[0]->token
                      ? node->children.items[0]->token->lexeme : "";
    EMITL(B(cg), "%s:", lbl);
}

 
/* Expression dispatch */
static void cg_expr(ELF32_Codegen *cg, const Ast_node *node) {
    if (!node) return;

    switch (node->kind) {

        case NODE_INT_LITERAL:
            EMITL(B(cg), "    mov eax, %s",
                  node->token ? node->token->lexeme : "0");
            break;

        case NODE_FLOAT_LITERAL:
        case NODE_FLOAT32_LITERAL:
            /* FPU not in CRASM backend; zero for now */
            EMITL(B(cg), "    ; float not supported by backend, using 0");
            EMITL(B(cg), "    xor eax, eax");
            break;

        case NODE_CHAR_LITERAL:
            if (node->token && node->token->lexeme && node->token->lexeme[0])
                EMITL(B(cg), "    mov eax, %d",
                      (unsigned char)node->token->lexeme[0]);
            else
                EMITL(B(cg), "    xor eax, eax");
            break;

        case NODE_STRING_LITERAL: {
            /* Intern the string and emit a load of its stack address.
               At runtime _craw_str_init has placed all string pointers
               into the table at [_start_frame - 4 - si*4].
               Since we can't access _start's frame from inside a function
               we use a simpler approach: push the string inline at the
               call site and let eax hold esp. */
            const char *raw = node->token ? node->token->lexeme : "";
            intern_string(cg, raw);  /* ensure it is in the table */
            int len = (int)strlen(raw);

            EMITL(B(cg), "    ; string literal \"%s\"", raw);
            /* Push null terminator first (stack grows down, null at highest address) */
            EMITL(B(cg), "    push 0");
            /* Push chars in reverse */
            for (int bi = len - 1; bi >= 0; bi--)
                EMITL(B(cg), "    push %u", (unsigned char)raw[bi]);
            /* eax = pointer to start of string on stack */
            EMITL(B(cg), "    mov eax, esp");
            break;
        }

        case NODE_IDENTIFIER: {
            if (!node->token || !node->token->lexeme) break;
            bool found = false;
            int offset = ELF32_SymTable_find(&cg->syms, node->token->lexeme, &found);
            if (found) {
                EMIT(B(cg), "    mov eax, "); emit_ebp_ref(B(cg), offset); ELF32_StrBuf_appendf(B(cg), "    ; %s\n", node->token->lexeme);
            } else {
                /* Check global variable table */
                int goff = global_find(cg, node->token->lexeme);
                if (goff >= 0) {
                    EMITL(B(cg), "    mov eax, [esi + %d]    ; global %s",
                          goff, node->token->lexeme);
                } else {
                    EMITL(B(cg), "    ; unresolved '%s' — did you mean a call?",
                          node->token->lexeme);
                    EMITL(B(cg), "    xor eax, eax");
                }
            }
            break;
        }

        case NODE_BINARY_OP:   cg_binary(cg, node); break;
        case NODE_UNARY_OP:    cg_unary(cg, node);  break;
        case NODE_ASSIGN:      cg_assign(cg, node);  break;
        case NODE_CALL:        cg_call(cg, node);    break;

        case NODE_FIELD_ACCESS: {
            /*
             * field access: obj.field
             * children[0] = object (identifier)
             * children[1] = field name (identifier)
             *
             * We find the struct type of the object from the symbol table,
             * look up the field offset, then load [ebp + obj_offset + field_offset].
             */
            if (node->children.size < 2) break;
            const Ast_node *obj   = node->children.items[0];
            const Ast_node *field = node->children.items[1];
            if (!obj->token || !field->token) break;

            const char *oname = obj->token->lexeme;
            const char *fname = field->token->lexeme;

            /* Find the struct base offset */
            bool found = false;
            int obj_off = 0;
            const ELF32_StructDef *sd = NULL;

            for (int vi = 0; vi < cg->syms.count; vi++) {
                const char *vn = cg->syms.vars[vi].name;
                const char *colon = strchr(vn, ':');
                size_t nlen = colon ? (size_t)(colon - vn) : strlen(vn);
                if (strncmp(vn, oname, nlen) == 0 && strlen(oname) == nlen) {
                    found = true;
                    obj_off = cg->syms.vars[vi].offset;
                    if (colon) sd = find_struct(cg, colon + 1);
                    break;
                }
            }

            if (!found) {
                EMITL(B(cg), "    ; ERROR: unknown variable '%s'", oname);
                cg->had_error = true;
                break;
            }
            if (!sd) {
                EMITL(B(cg), "    ; ERROR: '%s' is not a struct", oname);
                cg->had_error = true;
                break;
            }

            /* Find the field */
            const ELF32_StructField *sf = NULL;
            for (int fi = 0; fi < sd->nfields; fi++) {
                if (strcmp(sd->fields[fi].name, fname) == 0) {
                    sf = &sd->fields[fi];
                    break;
                }
            }
            if (!sf) {
                EMITL(B(cg), "    ; ERROR: no field '%s' in struct '%s'", fname, sd->name);
                cg->had_error = true;
                break;
            }

            /* Load: eax = [ebp + obj_offset + field_offset] */
            int total_off = obj_off + sf->offset;
            EMITL(B(cg), "    ; %s.%s  [ebp%+d]", oname, fname, total_off);
            EMIT(B(cg), "    mov eax, ");
            emit_ebp_ref(B(cg), total_off);
            ELF32_StrBuf_append(B(cg), "\n");
            break;
        }

        case NODE_INDEX:
            if (node->children.size >= 2) {
                cg_expr(cg, node->children.items[1]);  /* index → eax */
                EMITL(B(cg), "    push eax");
                cg_expr(cg, node->children.items[0]);  /* base → eax */
                EMITL(B(cg), "    pop ebx");
                EMITL(B(cg), "    imul ebx, 4");
                EMITL(B(cg), "    add eax, ebx");
                EMITL(B(cg), "    mov eax, [eax]");
            }
            break;

        case NODE_ARRAY_LITERAL: {
            /* Push elements right-to-left; eax = esp (base pointer) */
            EMITL(B(cg), "    ; array literal (%zu elements)", node->children.size);
            for (int i = (int)node->children.size - 1; i >= 0; i--) {
                cg_expr(cg, node->children.items[i]);
                EMITL(B(cg), "    push eax");
            }
            EMITL(B(cg), "    mov eax, esp");
            break;
        }

        case NODE_STRUCT_NEW:
            EMITL(B(cg), "    ; 'new' not yet implemented");
            EMITL(B(cg), "    xor eax, eax");
            break;

        default:
            EMITL(B(cg), "    ; unhandled expr node %d", (int)node->kind);
            break;
    }
}

 
/* Binary operators */
 

/*
 * Compile: left→eax, push; right→eax; pop ebx (=left).
 * eax = right operand, ebx = left operand.
 * Result in eax.
 */
static void cg_binary(ELF32_Codegen *cg, const Ast_node *node) {
    if (!node->token || node->children.size < 2) {
        cg_error(cg, "Malformed binary-op", node->token);
        return;
    }
    cg_expr(cg, node->children.items[0]);
    EMITL(B(cg), "    push eax");
    cg_expr(cg, node->children.items[1]);
    EMITL(B(cg), "    pop ebx");
    /* eax=right, ebx=left */

    enum TokenType op = node->token->tokenType;
    switch (op) {

        case Plus:
            EMITL(B(cg), "    add eax, ebx");
            break;
        case Minus:
            EMITL(B(cg), "    sub ebx, eax");
            EMITL(B(cg), "    mov eax, ebx");
            break;
        case Star:
            EMITL(B(cg), "    imul eax, ebx");
            break;

        case Slash:
            /* signed divide: ebx / eax -- quotient in eax
               cdq sign-extends eax into edx:eax before idiv */
            EMITL(B(cg), "    push edx");
            EMITL(B(cg), "    mov ecx, eax      ; divisor");
            EMITL(B(cg), "    mov eax, ebx      ; dividend");
            EMITL(B(cg), "    cdq               ; sign-extend eax into edx");
            EMITL(B(cg), "    idiv ecx          ; eax = quotient");
            EMITL(B(cg), "    pop edx");
            break;

        case Percent:
            /* remainder in edx after idiv */
            EMITL(B(cg), "    push edx");
            EMITL(B(cg), "    mov ecx, eax      ; divisor");
            EMITL(B(cg), "    mov eax, ebx      ; dividend");
            EMITL(B(cg), "    cdq               ; sign-extend eax into edx");
            EMITL(B(cg), "    idiv ecx          ; edx = remainder");
            EMITL(B(cg), "    mov eax, edx      ; return remainder");
            EMITL(B(cg), "    pop edx");
            break;

        case BitwiseAnd:  EMITL(B(cg), "    and eax, ebx");  break;
        case BitwiseOr:   EMITL(B(cg), "    or eax, ebx");   break;
        case BitwiseXor:  EMITL(B(cg), "    xor eax, ebx");  break;

        case LeftShift:
        case RightShift: {
            /* CRASM shl/shr only accept immediate operands.
               Check if the right operand is a compile-time integer literal. */
            const Ast_node *rhs = node->children.items[1];
            const char *shift_op = (op == LeftShift) ? "shl" : "shr";
            if (rhs->kind == NODE_INT_LITERAL && rhs->token) {
                /* Immediate known at compile-time: emit directly */
                EMITL(B(cg), "    %s ebx, %s", shift_op, rhs->token->lexeme);
            } else {
                /* Variable shift: not supported by CRASM backend emit 1 as fallback */
                fprintf(stderr,
                    "\e[33mCODEGEN WARNING:\e[0m variable shift count not supported "
                    "by CRASM backend; using shift-by-1 fallback\n");
                EMITL(B(cg), "    %s ebx, 1    ; WARNING: variable shift used shift-by-1", shift_op);
            }
            EMITL(B(cg), "    mov eax, ebx");
            break;
        }

        /* Logical short-circuit already evaluated both sides so we just
           combine the boolean results */
        case And: {
            char lf[32], ld[32];
            ELF32_Codegen_fresh_label(cg, lf, "and_f");
            ELF32_Codegen_fresh_label(cg, ld, "and_d");
            EMITL(B(cg), "    test ebx, ebx");
            EMITL(B(cg), "    je %s", lf);
            EMITL(B(cg), "    test eax, eax");
            EMITL(B(cg), "    je %s", lf);
            EMITL(B(cg), "    mov eax, 1");
            EMITL(B(cg), "    jmp %s", ld);
            EMITL(B(cg), "%s:", lf);
            EMITL(B(cg), "    xor eax, eax");
            EMITL(B(cg), "%s:", ld);
            break;
        }
        case Or: {
            char lt[32], ld[32];
            ELF32_Codegen_fresh_label(cg, lt, "or_t");
            ELF32_Codegen_fresh_label(cg, ld, "or_d");
            EMITL(B(cg), "    test ebx, ebx");
            EMITL(B(cg), "    jne %s", lt);
            EMITL(B(cg), "    test eax, eax");
            EMITL(B(cg), "    jne %s", lt);
            EMITL(B(cg), "    xor eax, eax");
            EMITL(B(cg), "    jmp %s", ld);
            EMITL(B(cg), "%s:", lt);
            EMITL(B(cg), "    mov eax, 1");
            EMITL(B(cg), "%s:", ld);
            break;
        }

        /* Comparisons 0 or 1 in eax */
        case Equal:
        case NotEqual:
        case LessThan:
        case GreaterThan:
        case LessEqual:
        case GreaterEqual: {
            char lt[32], ld[32];
            ELF32_Codegen_fresh_label(cg, lt, "cmp_t");
            ELF32_Codegen_fresh_label(cg, ld, "cmp_d");
            EMITL(B(cg), "    cmp ebx, eax");
            const char *jmp;
            switch(op) {
                case Equal:        jmp = "je";  break;
                case NotEqual:     jmp = "jne"; break;
                case LessThan:     jmp = "jl";  break;
                case GreaterThan:  jmp = "jg";  break;
                case LessEqual:    jmp = "jle"; break;
                case GreaterEqual: jmp = "jge"; break;
                default:           jmp = "je";  break;
            }
            EMITL(B(cg), "    %s %s", jmp, lt);
            EMITL(B(cg), "    xor eax, eax");
            EMITL(B(cg), "    jmp %s", ld);
            EMITL(B(cg), "%s:", lt);
            EMITL(B(cg), "    mov eax, 1");
            EMITL(B(cg), "%s:", ld);
            break;
        }

        default:
            EMITL(B(cg), "    ; unknown binary op %d", (int)op);
            break;
    }
}

 
/* Unary */
static void cg_unary(ELF32_Codegen *cg, const Ast_node *node) {
    if (!node->token || !node->children.size) return;
    cg_expr(cg, node->children.items[0]);

    switch (node->token->tokenType) {
        case Minus:
            EMITL(B(cg), "    neg eax");
            break;
        case Not: {
            char lf[32], ld[32];
            ELF32_Codegen_fresh_label(cg, lf, "not_f");
            ELF32_Codegen_fresh_label(cg, ld, "not_d");
            EMITL(B(cg), "    test eax, eax");
            EMITL(B(cg), "    jne %s", lf);
            EMITL(B(cg), "    mov eax, 1");
            EMITL(B(cg), "    jmp %s", ld);
            EMITL(B(cg), "%s:", lf);
            EMITL(B(cg), "    xor eax, eax");
            EMITL(B(cg), "%s:", ld);
            break;
        }
        case BitwiseNot:
            EMITL(B(cg), "    not eax");
            break;
        default:
            EMITL(B(cg), "    ; unknown unary op %d", (int)node->token->tokenType);
            break;
    }
}
 
/* Assignment */
static void cg_assign(ELF32_Codegen *cg, const Ast_node *node) {
    if (node->children.size < 2) return;

    /* Compile RHS - eax */
    cg_expr(cg, node->children.items[1]);

    const Ast_node *target = node->children.items[0];
    if (target->kind == NODE_IDENTIFIER && target->token) {
        bool found = false;
        int offset = ELF32_SymTable_find(&cg->syms, target->token->lexeme, &found);
        if (found) {
            EMIT(B(cg), "    mov "); emit_ebp_ref(B(cg), offset); ELF32_StrBuf_appendf(B(cg), ", eax    ; %s =\n", target->token->lexeme);
        } else {
            int goff = global_find(cg, target->token->lexeme);
            if (goff >= 0) {
                EMITL(B(cg), "    mov [esi + %d], eax    ; global %s =",
                      goff, target->token->lexeme);
            } else {
                EMITL(B(cg), "    ; ERROR: undeclared '%s'", target->token->lexeme);
                cg->had_error = true;
            }
        }
    } else if (target->kind == NODE_INDEX && target->children.size >= 2) {
        EMITL(B(cg), "    push eax");
        cg_expr(cg, target->children.items[1]);  /* index */
        EMITL(B(cg), "    push eax");
        cg_expr(cg, target->children.items[0]);  /* base */
        EMITL(B(cg), "    pop ebx");              /* index */
        EMITL(B(cg), "    imul ebx, 4");
        EMITL(B(cg), "    add eax, ebx");         /* address */
        EMITL(B(cg), "    pop ebx");              /* value */
        EMITL(B(cg), "    mov [eax], ebx");
    } else if (target->kind == NODE_FIELD_ACCESS && target->children.size >= 2) {
        /* struct field assignment: obj.field = value (eax) */
        const char *oname = target->children.items[0]->token
                            ? target->children.items[0]->token->lexeme : "";
        const char *fname = target->children.items[1]->token
                            ? target->children.items[1]->token->lexeme : "";

        /* Find object in sym table */
        int obj_off = 0;
        const ELF32_StructDef *sd = NULL;
        for (int vi = 0; vi < cg->syms.count; vi++) {
            const char *vn = cg->syms.vars[vi].name;
            const char *colon = strchr(vn, ':');
            size_t nlen = colon ? (size_t)(colon - vn) : strlen(vn);
            if (strncmp(vn, oname, nlen) == 0 && strlen(oname) == nlen) {
                obj_off = cg->syms.vars[vi].offset;
                if (colon) sd = find_struct(cg, colon + 1);
                break;
            }
        }

        if (!sd) {
            EMITL(B(cg), "    ; ERROR: '%s' is not a struct or not found", oname);
            cg->had_error = true;
        } else {
            const ELF32_StructField *sf = NULL;
            for (int fi = 0; fi < sd->nfields; fi++) {
                if (strcmp(sd->fields[fi].name, fname) == 0) { sf = &sd->fields[fi]; break; }
            }
            if (!sf) {
                EMITL(B(cg), "    ; ERROR: no field '%s' in '%s'", fname, sd->name);
                cg->had_error = true;
            } else {
                int total_off = obj_off + sf->offset;
                EMITL(B(cg), "    ; %s.%s = eax  [ebp%+d]", oname, fname, total_off);
                EMIT(B(cg), "    mov ");
                emit_ebp_ref(B(cg), total_off);
                ELF32_StrBuf_append(B(cg), ", eax\n");
            }
        }
    } else {
        EMITL(B(cg), "    ; complex assignment target not yet supported");
    }
}

 
/* Function call (cdecl) */
static void cg_call(ELF32_Codegen *cg, const Ast_node *node) {
    if (!node->children.size) return;

    const Ast_node *callee = node->children.items[0];
    const char *fname = (callee->token && callee->token->lexeme)
                        ? callee->token->lexeme : "unknown";
    size_t argc = node->children.size - 1;

    EMITL(B(cg), "    ; call %s (%zu args)", fname, argc);

    /* Push arguments right-to-left */
    for (int i = (int)argc; i >= 1; i--) {
        cg_expr(cg, node->children.items[i]);
        EMITL(B(cg), "    push eax");
    }

    EMITL(B(cg), "    call %s", fname);

    if (argc > 0)
        EMITL(B(cg), "    add esp, %d    ; pop %zu arg(s)",
              (int)(argc * 4), argc);
    /* Return value already in eax */
}