#include "asm.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOK 512

typedef enum {
    TOK_IDENT,
    TOK_NUMBER,
    TOK_STRING,
    TOK_COMMA,
    TOK_COLON,
    TOK_NEWLINE,
    TOK_EOF,
    TOK_ERROR,
} TokenKind;

typedef struct {
    TokenKind kind;
    char text[MAX_TOK];
    uint32_t num;
    int line;
} Token;

typedef struct {
    const char *src;
    size_t pos;
    size_t len;
    int line;
} Lexer;

typedef enum {
    AARG_NONE,
    AARG_NUMBER,
    AARG_GOD,
    AARG_LABEL,
    AARG_STRING,
} AArgType;

typedef struct {
    AArgType type;
    uint32_t num;
    char str[MAX_TOK];
} AArg;

typedef struct {
    int opcode;
    AArg a1, a2;
    int line;
} AInstr;

typedef struct {
    char name[MAX_TOK];
    uint32_t addr;
} LabelEntry;

static void lex_init(Lexer *l, const char *src, size_t len) {
    l->src = src;
    l->pos = 0;
    l->len = len;
    l->line = 1;
}

static char lpeek(Lexer *l) {
    return (l->pos < l->len) ? l->src[l->pos] : '\0';
}

static char ladv(Lexer *l) {
    char c = l->src[l->pos++];
    if (c == '\n') l->line++;
    return c;
}

static Token lex_next(Lexer *l) {
    Token t = {0};

    while (l->pos < l->len && (lpeek(l) == ' ' || lpeek(l) == '\t' || lpeek(l) == '\r'))
        ladv(l);

    if (lpeek(l) == ';') {
        while (l->pos < l->len && lpeek(l) != '\n')
            ladv(l);
    }

    t.line = l->line;

    if (l->pos >= l->len) { t.kind = TOK_EOF; return t; }

    char c = lpeek(l);

    if (c == '\n') { ladv(l); t.kind = TOK_NEWLINE; return t; }
    if (c == ',')  { ladv(l); t.kind = TOK_COMMA;   return t; }
    if (c == ':')  { ladv(l); t.kind = TOK_COLON;   return t; }

    if (c == '"') {
        ladv(l);
        int i = 0;
        while (l->pos < l->len && lpeek(l) != '"' && lpeek(l) != '\n') {
            char ch = ladv(l);
            if (ch == '\\' && l->pos < l->len) {
                char esc = ladv(l);
                switch (esc) {
                    case 'n':  ch = '\n'; break;
                    case 't':  ch = '\t'; break;
                    case '"':  ch = '"';  break;
                    case '\\': ch = '\\'; break;
                    default:   ch = esc;  break;
                }
            } else {
                // ch already set
            }
            if (i < MAX_TOK - 1) t.text[i++] = ch;
        }
        t.text[i] = '\0';
        if (lpeek(l) == '"') ladv(l);
        t.kind = TOK_STRING;
        return t;
    }

    if (isdigit((unsigned char)c)) {
        int i = 0;
        while (l->pos < l->len && isdigit((unsigned char)lpeek(l))) {
            if (i < MAX_TOK - 1) t.text[i++] = ladv(l);
            else ladv(l);
        }
        t.text[i] = '\0';
        t.num = (uint32_t)strtoul(t.text, NULL, 10);
        t.kind = TOK_NUMBER;
        return t;
    }

    if (isalpha((unsigned char)c) || c == '_') {
        int i = 0;
        while (l->pos < l->len && (isalnum((unsigned char)lpeek(l)) || lpeek(l) == '_')) {
            if (i < MAX_TOK - 1) t.text[i++] = ladv(l);
            else ladv(l);
        }
        t.text[i] = '\0';
        t.kind = TOK_IDENT;
        return t;
    }

    t.kind = TOK_ERROR;
    t.text[0] = ladv(l);
    t.text[1] = '\0';
    return t;
}

static void to_upper(const char *in, char *out, size_t size) {
    size_t i;
    for (i = 0; in[i] && i < size - 1; i++)
        out[i] = (char)toupper((unsigned char)in[i]);
    out[i] = '\0';
}

static int parse_god(const char *name) {
    char u[64];
    to_upper(name, u, sizeof(u));
    if (strcmp(u, "ARES")     == 0) return GOD_ARES;
    if (strcmp(u, "HERMES")   == 0) return GOD_HERMES;
    if (strcmp(u, "ATHENA")   == 0) return GOD_ATHENA;
    if (strcmp(u, "ZEUS")     == 0) return GOD_ZEUS;
    if (strcmp(u, "HERA")     == 0) return GOD_HERA;
    if (strcmp(u, "POSEIDON") == 0) return GOD_POSEIDON;
    if (strcmp(u, "APOLLO")   == 0) return GOD_APOLLO;
    if (strcmp(u, "HADES")    == 0) return GOD_HADES;
    return -1;
}

static int parse_opcode(const char *name) {
    char u[64];
    to_upper(name, u, sizeof(u));
    if (strcmp(u, "BORN")     == 0) return OP_BORN;
    if (strcmp(u, "SLAY")     == 0) return OP_SLAY;
    if (strcmp(u, "GLORY")    == 0) return OP_GLORY;
    if (strcmp(u, "QUEST")    == 0) return OP_QUEST;
    if (strcmp(u, "TRIAL")    == 0) return OP_TRIAL;
    if (strcmp(u, "PROPHECY") == 0) return OP_PROPHECY;
    if (strcmp(u, "ORACLE")   == 0) return OP_ORACLE;
    if (strcmp(u, "HUBRIS")   == 0) return OP_HUBRIS;
    if (strcmp(u, "EXILE")    == 0) return OP_EXILE;
    if (strcmp(u, "RETURN")   == 0) return OP_RETURN;
    if (strcmp(u, "ODYSSEY")  == 0) return OP_ODYSSEY;
    if (strcmp(u, "SCROLL")   == 0) return OP_SCROLL;
    if (strcmp(u, "RECITE")   == 0) return OP_RECITE;
    if (strcmp(u, "SET")      == 0) return OP_SET;
    if (strcmp(u, "TRAGEDY")  == 0) return OP_TRAGEDY;
    if (strcmp(u, "EPITAPH")  == 0) return OP_EPITAPH;
    return -1;
}

typedef struct {
    Lexer lex;
    Token cur;
    Token peek;
    int peaked;
    AInstr instrs[MAX_INSTRS];
    int n_instrs;
    LabelEntry labels[MAX_LABELS];
    int n_labels;
    char strpool[MAX_STRPOOL];
    int strpool_len;
    int had_error;
} Parser;

static void p_error(Parser *p, int line, const char *fmt, ...) {
    va_list ap;
    fprintf(stderr, "error: line %d: ", line);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    p->had_error = 1;
}

static void skip_to_newline(Parser *p) {
    Token t;
    do {
        t = lex_next(&p->lex);
    } while (t.kind != TOK_NEWLINE && t.kind != TOK_EOF);
}

static Token expect_ident(Parser *p) {
    Token t = lex_next(&p->lex);
    if (t.kind != TOK_IDENT)
        p_error(p, t.line, "expected identifier, got '%s'", t.text);
    return t;
}

static Token expect_number(Parser *p) {
    Token t = lex_next(&p->lex);
    if (t.kind != TOK_NUMBER)
        p_error(p, t.line, "expected number, got '%s'", t.text);
    return t;
}

static void expect_comma(Parser *p) {
    Token t = lex_next(&p->lex);
    if (t.kind != TOK_COMMA)
        p_error(p, t.line, "expected ','");
}

static uint32_t add_string(Parser *p, const char *s) {
    uint32_t off = (uint32_t)p->strpool_len;
    size_t slen = strlen(s) + 1;
    if (p->strpool_len + (int)slen > MAX_STRPOOL) {
        fprintf(stderr, "error: string pool overflow\n");
        p->had_error = 1;
        return 0;
    }
    memcpy(p->strpool + p->strpool_len, s, slen);
    p->strpool_len += (int)slen;
    return off;
}

static void add_label(Parser *p, const char *name, uint32_t addr, int line) {
    char u[MAX_TOK];
    to_upper(name, u, sizeof(u));
    for (int i = 0; i < p->n_labels; i++) {
        if (strcmp(p->labels[i].name, u) == 0) {
            p_error(p, line, "duplicate label '%s'", name);
            return;
        }
    }
    if (p->n_labels >= MAX_LABELS) {
        p_error(p, line, "too many labels");
        return;
    }
    snprintf(p->labels[p->n_labels].name, MAX_TOK, "%s", u);
    p->labels[p->n_labels].addr = addr;
    p->n_labels++;
}

static int find_label(Parser *p, const char *name, int line) {
    char u[MAX_TOK];
    to_upper(name, u, sizeof(u));
    for (int i = 0; i < p->n_labels; i++) {
        if (strcmp(p->labels[i].name, u) == 0)
            return (int)p->labels[i].addr;
    }
    p_error(p, line, "undefined label '%s'", name);
    return 0;
}

static void emit(Parser *p, int opcode, AArg a1, AArg a2, int line) {
    if (p->n_instrs >= MAX_INSTRS) {
        p_error(p, line, "too many instructions");
        return;
    }
    AInstr *ins = &p->instrs[p->n_instrs++];
    ins->opcode = opcode;
    ins->a1 = a1;
    ins->a2 = a2;
    ins->line = line;
}

static AArg arg_none(void)         { AArg a = {0}; a.type = AARG_NONE; return a; }
static AArg arg_num(uint32_t n)    { AArg a = {0}; a.type = AARG_NUMBER; a.num = n; return a; }
static AArg arg_god(int g)         { AArg a = {0}; a.type = AARG_GOD; a.num = (uint32_t)g; return a; }
static AArg arg_label(const char *s) { AArg a = {0}; a.type = AARG_LABEL; snprintf(a.str, MAX_TOK, "%s", s); return a; }
static AArg arg_str(const char *s) { AArg a = {0}; a.type = AARG_STRING; snprintf(a.str, MAX_TOK, "%s", s); return a; }

static void parse_all(Parser *p) {
    while (1) {
        Token t = lex_next(&p->lex);

        if (t.kind == TOK_EOF) break;
        if (t.kind == TOK_NEWLINE) continue;

        if (t.kind == TOK_ERROR) {
            p_error(p, t.line, "unexpected character '%s'", t.text);
            continue;
        }

        if (t.kind != TOK_IDENT) {
            p_error(p, t.line, "expected instruction or label");
            skip_to_newline(p);
            continue;
        }

        // peek next token to check for label
        Token peek = lex_next(&p->lex);
        if (peek.kind == TOK_COLON) {
            add_label(p, t.text, (uint32_t)p->n_instrs, t.line);
            continue;
        }

        // Not a label: it's an instruction. Re-process peek.
        // We need to "put back" peek. Simplest: handle specially.
        // peek is next token after ident. For all instructions, after
        // the instruction name comes either newline, ident, number, or string.
        // We can just re-inject it by saving it.
        // Since our lexer is not peek-capable, we handle this by parsing
        // the first arg from peek directly.

        int op = parse_opcode(t.text);
        if (op < 0) {
            p_error(p, t.line, "unknown instruction '%s'", t.text);
            // consume peek and rest of line
            if (peek.kind != TOK_NEWLINE && peek.kind != TOK_EOF)
                skip_to_newline(p);
            continue;
        }

        int line = t.line;
        Token t2 = peek; // first token after instruction name

        int g;
        switch (op) {
        case OP_BORN: {
            if (t2.kind != TOK_NUMBER)
                p_error(p, line, "BORN: expected number");
            uint32_t val = t2.num;
            if (val > MASK26)
                p_error(p, line, "BORN value %u exceeds 26-bit limit", val);
            AArg a1 = arg_num((val >> 13) & MASK13);
            AArg a2 = arg_num(val & MASK13);
            emit(p, op, a1, a2, line);
            skip_to_newline(p);
            break;
        }
        case OP_SLAY:
        case OP_QUEST:
        case OP_EXILE:
        case OP_RETURN: {
            if (t2.kind != TOK_IDENT)
                p_error(p, line, "%s: expected god name", t.text);
            g = parse_god(t2.text);
            if (g < 0) p_error(p, line, "unknown god '%s'", t2.text);
            emit(p, op, arg_god(g < 0 ? 0 : g), arg_none(), line);
            skip_to_newline(p);
            break;
        }
        case OP_GLORY:
        case OP_HUBRIS:
        case OP_SCROLL:
        case OP_RECITE: {
            // t2 should be newline or EOF; if not, warn
            if (t2.kind != TOK_NEWLINE && t2.kind != TOK_EOF)
                skip_to_newline(p);
            emit(p, op, arg_none(), arg_none(), line);
            break;
        }
        case OP_TRIAL: {
            if (t2.kind != TOK_IDENT)
                p_error(p, line, "TRIAL: expected god name");
            g = parse_god(t2.text);
            if (g < 0) p_error(p, line, "unknown god '%s'", t2.text);
            expect_comma(p);
            Token lt = expect_ident(p);
            emit(p, op, arg_god(g < 0 ? 0 : g), arg_label(lt.text), line);
            skip_to_newline(p);
            break;
        }
        case OP_PROPHECY:
        case OP_ORACLE: {
            if (t2.kind != TOK_IDENT)
                p_error(p, line, "%s: expected label", t.text);
            emit(p, op, arg_label(t2.text), arg_none(), line);
            skip_to_newline(p);
            break;
        }
        case OP_ODYSSEY: {
            if (t2.kind != TOK_IDENT)
                p_error(p, line, "ODYSSEY: expected label");
            char lbl[MAX_TOK];
            snprintf(lbl, MAX_TOK, "%s", t2.text);
            expect_comma(p);
            Token nt = expect_number(p);
            emit(p, op, arg_label(lbl), arg_num(nt.num), line);
            skip_to_newline(p);
            break;
        }
        case OP_SET: {
            if (t2.kind != TOK_IDENT)
                p_error(p, line, "SET: expected god name");
            g = parse_god(t2.text);
            if (g < 0) p_error(p, line, "unknown god '%s'", t2.text);
            expect_comma(p);
            Token vt = expect_number(p);
            if (vt.num > MASK13)
                p_error(p, line, "SET value %u exceeds 13-bit limit", vt.num);
            emit(p, op, arg_god(g < 0 ? 0 : g), arg_num(vt.num & MASK13), line);
            skip_to_newline(p);
            break;
        }
        case OP_TRAGEDY:
        case OP_EPITAPH: {
            if (t2.kind != TOK_STRING)
                p_error(p, line, "%s: expected string", t.text);
            emit(p, op, arg_str(t2.text), arg_none(), line);
            skip_to_newline(p);
            break;
        }
        }
    }
}

static void write_u32le(FILE *f, uint32_t v) {
    uint8_t b[4] = {v & 0xFF, (v >> 8) & 0xFF, (v >> 16) & 0xFF, (v >> 24) & 0xFF};
    fwrite(b, 1, 4, f);
}

int assemble_file(const char *src_path, const char *dst_path) {
    FILE *in = fopen(src_path, "rb");
    if (!in) {
        fprintf(stderr, "error: cannot open '%s'\n", src_path);
        return 1;
    }

    fseek(in, 0, SEEK_END);
    long fsize = ftell(in);
    fseek(in, 0, SEEK_SET);

    char *buf = malloc((size_t)fsize + 1);
    if (!buf) { fclose(in); fprintf(stderr, "error: out of memory\n"); return 1; }
    fread(buf, 1, (size_t)fsize, in);
    buf[fsize] = '\0';
    fclose(in);

    Parser *p = calloc(1, sizeof(Parser));
    if (!p) { free(buf); fprintf(stderr, "error: out of memory\n"); return 1; }
    lex_init(&p->lex, buf, (size_t)fsize);

    parse_all(p);
    free(buf);

    if (p->had_error) { free(p); return 1; }

    // Resolve labels and strings, build final words
    uint32_t *words = malloc(sizeof(uint32_t) * (size_t)p->n_instrs);
    if (!words) { free(p); fprintf(stderr, "error: out of memory\n"); return 1; }

    for (int i = 0; i < p->n_instrs; i++) {
        AInstr *ins = &p->instrs[i];
        uint32_t a1 = 0, a2 = 0;

        switch (ins->a1.type) {
        case AARG_NONE:   a1 = 0; break;
        case AARG_NUMBER: a1 = ins->a1.num & MASK13; break;
        case AARG_GOD:    a1 = ins->a1.num & 0x7; break;
        case AARG_LABEL: {
            int addr = find_label(p, ins->a1.str, ins->line);
            if (p->had_error) { free(words); free(p); return 1; }
            a1 = (uint32_t)addr & MASK13;
            break;
        }
        case AARG_STRING: {
            uint32_t off = add_string(p, ins->a1.str);
            a1 = off & MASK13;
            break;
        }
        }

        switch (ins->a2.type) {
        case AARG_NONE:   a2 = 0; break;
        case AARG_NUMBER: a2 = ins->a2.num & MASK13; break;
        case AARG_GOD:    a2 = ins->a2.num & 0x7; break;
        case AARG_LABEL: {
            int addr = find_label(p, ins->a2.str, ins->line);
            if (p->had_error) { free(words); free(p); return 1; }
            a2 = (uint32_t)addr & MASK13;
            break;
        }
        case AARG_STRING: {
            uint32_t off = add_string(p, ins->a2.str);
            a2 = off & MASK13;
            break;
        }
        }

        words[i] = ((uint32_t)ins->opcode << 26) | (a1 << 13) | a2;
    }

    if (p->had_error) { free(words); free(p); return 1; }

    FILE *out = fopen(dst_path, "wb");
    if (!out) {
        fprintf(stderr, "error: cannot write '%s'\n", dst_path);
        free(words); free(p);
        return 1;
    }

    write_u32le(out, POEM_MAGIC);
    write_u32le(out, (uint32_t)p->n_instrs);
    for (int i = 0; i < p->n_instrs; i++)
        write_u32le(out, words[i]);
    if (p->strpool_len > 0)
        fwrite(p->strpool, 1, (size_t)p->strpool_len, out);

    fclose(out);
    free(words);
    free(p);
    return 0;
}
