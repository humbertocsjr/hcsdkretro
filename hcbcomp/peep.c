#include "peep.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

/* ── Instruction representation ─────────────────────────────── */

#define PEEP_MAX_LINE 512
#define PEEP_MAX_ARGS 16
#define PEEP_WINDOW   12

typedef struct {
    char raw[PEEP_MAX_LINE];    /* original line including leading whitespace */
    const char *label;          /* pointer into raw if line is a label */
    const char *comment;        /* pointer into raw if line is a comment */
    char opcode[64];
    char args[PEEP_MAX_ARGS][64];
    int  nargs;
    int  is_inst;               /* 1 = real instruction, 0 = label/comment/directive */
} peep_line_t;

typedef struct {
    peep_line_t *lines;
    int count;
    int cap;
} peep_seq_t;

/* ── Line parser ────────────────────────────────────────────── */

static const char *skip_space(const char *p)
{
    while (*p == ' ' || *p == '\t') p++;
    return p;
}

static int parse_line(peep_line_t *pl, const char *raw)
{
    memset(pl, 0, sizeof(*pl));
    strncpy(pl->raw, raw, PEEP_MAX_LINE - 1);
    pl->raw[PEEP_MAX_LINE - 1] = 0;

    const char *p = raw;

    /* skip leading whitespace (tab) */
    if (*p == '\t') p++;
    else if (*p == ' ' && *(p+1) == ' ' && *(p+2) == ' ') p += 3;
    else { pl->is_inst = 0; return 0; }

    p = skip_space(p);

    /* label? */
    if (isalpha(*p) || *p == '_' || *p == '.') {
        const char *end = p;
        while (isalnum(*end) || *end == '_' || *end == '.') end++;
        if (*end == ':') {
            pl->label = pl->raw + (p - raw);
            pl->is_inst = 0;
            return 0;
        }
    }

    /* comment? */
    if (*p == ';') {
        pl->comment = pl->raw + (p - raw);
        pl->is_inst = 0;
        return 0;
    }

    /* directive? (.section, global, extern, dw, ds, db) */
    if (*p == '.') {
        pl->is_inst = 0;
        return 0;
    }

    /* must be an instruction */
    pl->is_inst = 1;

    /* opcode */
    const char *start = p;
    while (*p && !isspace(*p) && *p != '\t') p++;
    int len = p - start;
    if (len >= 63) len = 63;
    memcpy(pl->opcode, start, len);
    pl->opcode[len] = 0;

    /* arguments */
    pl->nargs = 0;
    while (*p && pl->nargs < PEEP_MAX_ARGS) {
        p = skip_space(p);
        if (*p == 0 || *p == '\n' || *p == '\r') break;
        if (*p == ';') break; /* comment after instruction */
        start = p;
        while (*p && *p != ',' && *p != ';' && *p != '\n' && *p != '\r') p++;
        len = p - start;
        /* trim trailing space */
        while (len > 0 && (start[len-1] == ' ' || start[len-1] == '\t')) len--;
        if (len >= 63) len = 63;
        memcpy(pl->args[pl->nargs], start, len);
        pl->args[pl->nargs][len] = 0;
        pl->nargs++;
        if (*p == ',') p++;
    }

    return 1;
}

/* Parse argument: extract integer constant if present. Returns 1 if constant. */
static int parse_arg_int(const char *arg, int *val)
{
    if (!arg || !*arg) return 0;
    int neg = 1;
    if (*arg == '-') { neg = -1; arg++; }
    else if (*arg == '+') arg++;
    if (!*arg) return 0;
    int v = 0;
    if (arg[0] == '0' && (arg[1] == 'x' || arg[1] == 'X')) {
        arg += 2;
        while (isxdigit(*arg)) {
            v = v * 16 + (isdigit(*arg) ? *arg - '0' : (toupper(*arg) - 'A' + 10));
            arg++;
        }
    } else {
        while (isdigit(*arg)) {
            v = v * 10 + (*arg - '0');
            arg++;
        }
    }
    if (*arg) return 0; /* extra chars */
    *val = neg * v;
    return 1;
}

/* ── Sequence buffer ────────────────────────────────────────── */

static void seq_init(peep_seq_t *seq)
{
    seq->cap = 1024;
    seq->lines = (peep_line_t *)malloc(seq->cap * sizeof(peep_line_t));
    seq->count = 0;
}

static void seq_add(peep_seq_t *seq, const char *raw)
{
    if (seq->count >= seq->cap) {
        seq->cap *= 2;
        seq->lines = (peep_line_t *)realloc(seq->lines, seq->cap * sizeof(peep_line_t));
    }
    parse_line(&seq->lines[seq->count], raw);
    seq->count++;
}

static void seq_free(peep_seq_t *seq)
{
    free(seq->lines);
    seq->lines = NULL;
    seq->count = 0;
    seq->cap = 0;
}

/* ── Helper: emit replacement lines into a shifted buffer ────── */

static void emit_repl(peep_line_t *repl, int *pn, const char *fmt, ...)
{
    char buf[PEEP_MAX_LINE];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    parse_line(&repl[*pn], buf);
    (*pn)++;
}

/* ── Pattern: check opcode match ─────────────────────────────── */

static int op_is(const peep_line_t *pl, const char *op)
{
    return pl->is_inst && !strcmp(pl->opcode, op);
}

/* Match arg exactly */
static int arg_is(const peep_line_t *pl, int idx, const char *val)
{
    if (idx >= pl->nargs) return 0;
    return !strcmp(pl->args[idx], val);
}

/* Match opcode + nargs */
static int op_args(const peep_line_t *pl, const char *op, int n)
{
    return op_is(pl, op) && pl->nargs == n;
}

/* ───────────────────────────────────────────────────────────────
 * Z80 Peephole Patterns
 * ─────────────────────────────────────────────────────────────── */

/* Z80 Pattern 1: load local var with computed offset → direct ix load
 *
 * ld hl, CONST
 * ex de, hl
 * push ix
 * pop hl
 * add hl, de
 * ld a, [hl]
 * inc hl
 * ld h, [hl]
 * ld l, a
 *
 * →  ld l, [ix+CONST]        (note: CONST is already the signed offset ix+CONST)
 *     ld h, [ix+CONST+1]
 */
static int z80_match_local_load(peep_line_t *w, peep_line_t *repl)
{
    if (w[0].nargs != 2) return 0;  /* ld hl, CONST */
    if (!op_is(&w[0], "ld")) return 0;
    if (strcmp(w[0].args[0], "hl")) return 0;
    int off;
    if (!parse_arg_int(w[0].args[1], &off)) return 0;
    if (!op_is(&w[1], "ex") || strcmp(w[1].args[0], "de") || strcmp(w[1].args[1], "hl")) return 0;
    if (!op_is(&w[2], "push") || strcmp(w[2].args[0], "ix")) return 0;
    if (!op_is(&w[3], "pop") || strcmp(w[3].args[0], "hl")) return 0;
    if (!op_is(&w[4], "add") || strcmp(w[4].args[0], "hl") || strcmp(w[4].args[1], "de")) return 0;
    if (!op_is(&w[5], "ld") || strcmp(w[5].args[0], "a") || strcmp(w[5].args[1], "[hl]")) return 0;
    if (!op_is(&w[6], "inc") || strcmp(w[6].args[0], "hl")) return 0;
    if (!op_is(&w[7], "ld") || strcmp(w[7].args[0], "h") || strcmp(w[7].args[1], "[hl]")) return 0;
    if (!op_is(&w[8], "ld") || strcmp(w[8].args[0], "l") || strcmp(w[8].args[1], "a")) return 0;

    int n = 0;
    emit_repl(repl, &n, "\tld l, [ix%+d]", off);
    emit_repl(repl, &n, "\tld h, [ix%+d]", off + 1);
    return n;
}

/* Z80 Pattern 2: load local var with computed offset — altern. reg order
 *
 * ld hl, CONST
 * ex de, hl
 * push ix
 * pop hl
 * add hl, de
 * ld e, [hl]
 * inc hl
 * ld d, [hl]
 * ex de, hl
 *
 * →  ld l, [ix+CONST]
 *     ld h, [ix+CONST+1]
 */
static int z80_match_local_load2(peep_line_t *w, peep_line_t *repl)
{
    if (w[0].nargs != 2) return 0;
    if (!op_is(&w[0], "ld") || strcmp(w[0].args[0], "hl")) return 0;
    int off;
    if (!parse_arg_int(w[0].args[1], &off)) return 0;
    if (!op_is(&w[1], "ex") || strcmp(w[1].args[0], "de") || strcmp(w[1].args[1], "hl")) return 0;
    if (!op_is(&w[2], "push") || strcmp(w[2].args[0], "ix")) return 0;
    if (!op_is(&w[3], "pop") || strcmp(w[3].args[0], "hl")) return 0;
    if (!op_is(&w[4], "add") || strcmp(w[4].args[0], "hl") || strcmp(w[4].args[1], "de")) return 0;
    if (!op_is(&w[5], "ld") || strcmp(w[5].args[0], "e") || strcmp(w[5].args[1], "[hl]")) return 0;
    if (!op_is(&w[6], "inc") || strcmp(w[6].args[0], "hl")) return 0;
    if (!op_is(&w[7], "ld") || strcmp(w[7].args[0], "d") || strcmp(w[7].args[1], "[hl]")) return 0;
    if (!op_is(&w[8], "ex") || strcmp(w[8].args[0], "de") || strcmp(w[8].args[1], "hl")) return 0;

    int n = 0;
    emit_repl(repl, &n, "\tld l, [ix%+d]", off);
    emit_repl(repl, &n, "\tld h, [ix%+d]", off + 1);
    return n;
}

/* Z80 Pattern 3a: local address computation alone (no deref), used before push
 * ld hl, CONST
 * ex de, hl
 * push ix
 * pop hl
 * add hl, de
 * push hl           (or store)
 *
 * → lea equivalent is just computing ix+offset.
 * We can't directly do `lea hl, [ix+CONST]` but we can do:
 *   push ix
 *   pop hl
 *   ld de, CONST
 *   add hl, de
 * That's still 4 instructions instead of 5. But CONST is negative.
 * Better: since CONST = -(offset*2+2), we can compute directly:
 *   ld hl, CONST
 *   add hl, ix   — but Z80 has no add hl, ix!
 *
 * Actually we can push ix; pop hl which gives hl=ix. Then:
 *   ld de, CONST (if we need the value in DE)
 * But ld de, CONST is 3 bytes + push ix/pop hl is 2 = 5. Same.
 * No significant optimization possible. Skip this pattern.
 */

/* Z80 Pattern 3: store to local var (complete: compute addr, push, val, pop+store)
 *
 * This is too complex for peephole because value computation is in between.
 * We handle just the prefix computation separately (if no optimization, skip).
 */

/* Z80 Pattern 4: push/pop copy — de-dup
 * push hl; pop de  →  ex de, hl  (but this changes DE which might be in use)
 * Actually `push hl; pop de` sets DE=HL and preserves DE on stack.
 * `ex de, hl` swaps DE and HL. Not equivalent.
 * Skip.
 */

/* Z80 Pattern 5: redundant ld after same value is already there
 * ld hl, X; push hl; pop de; ld hl, X  →  ld hl, X; push hl; pop de
 * (second ld hl, X is redundant)
 */
static int z80_match_redundant_load(peep_line_t *w, peep_line_t *repl)
{
    /* Match: ld hl, X; ... [ld hl, X again when HL unchanged] */
    /* Too complex for peephole with variable middle. Skip for now. */
    return 0;
}

/* Z80 Pattern 5: store immediate to local var with computed offset
 *
 * ld hl, CONST
 * ex de, hl
 * push ix
 * pop hl
 * add hl, de
 * push hl
 * ld hl, IMM
 * pop de
 * ex de, hl
 * ld [hl], e
 * inc hl
 * ld [hl], d
 *
 * →  ld hl, IMM
 *     ld [ix+CONST], l
 *     ld [ix+CONST+1], h
 */
static int z80_match_local_store_imm(peep_line_t *w, peep_line_t *repl)
{
    if (w[0].nargs != 2) return 0;
    if (!op_is(&w[0], "ld") || strcmp(w[0].args[0], "hl")) return 0;
    int off;
    if (!parse_arg_int(w[0].args[1], &off)) return 0;
    if (!op_is(&w[1], "ex") || strcmp(w[1].args[0], "de") || strcmp(w[1].args[1], "hl")) return 0;
    if (!op_is(&w[2], "push") || strcmp(w[2].args[0], "ix")) return 0;
    if (!op_is(&w[3], "pop") || strcmp(w[3].args[0], "hl")) return 0;
    if (!op_is(&w[4], "add") || strcmp(w[4].args[0], "hl") || strcmp(w[4].args[1], "de")) return 0;
    if (!op_is(&w[5], "push") || strcmp(w[5].args[0], "hl")) return 0;
    /* ld hl, IMM */
    if (!op_is(&w[6], "ld") || strcmp(w[6].args[0], "hl")) return 0;
    int imm;
    if (!parse_arg_int(w[6].args[1], &imm)) return 0;
    if (!op_is(&w[7], "pop") || strcmp(w[7].args[0], "de")) return 0;
    if (!op_is(&w[8], "ex") || strcmp(w[8].args[0], "de") || strcmp(w[8].args[1], "hl")) return 0;
    if (!op_is(&w[9], "ld") || strcmp(w[9].args[0], "[hl]") || strcmp(w[9].args[1], "e")) return 0;
    if (!op_is(&w[10], "inc") || strcmp(w[10].args[0], "hl")) return 0;
    if (!op_is(&w[11], "ld") || strcmp(w[11].args[0], "[hl]") || strcmp(w[11].args[1], "d")) return 0;

    int n = 0;
    emit_repl(repl, &n, "\tld hl, %d", imm);
    emit_repl(repl, &n, "\tld [ix%+d], l", off);
    emit_repl(repl, &n, "\tld [ix%+d], h", off + 1);
    return n;
}

/* ── Apply Z80 patterns ─────────────────────────────────────── */

static int z80_replace(peep_line_t *window, int wcount, peep_line_t *repl)
{
    if (wcount == 9) {
        int n = z80_match_local_load(window, repl);
        if (n > 0) return n;
        n = z80_match_local_load2(window, repl);
        if (n > 0) return n;
    }
    if (wcount == 12) {
        int n = z80_match_local_store_imm(window, repl);
        if (n > 0) return n;
    }
    return 0;
}

/* ───────────────────────────────────────────────────────────────
 * 8086 Peephole Patterns
 * ─────────────────────────────────────────────────────────────── */

/* 8086 Pattern 1: mov ax, LABEL; mov bx, ax; mov ax, [bx]  →  mov ax, [LABEL] */
static int i86_match_label_deref(peep_line_t *w, peep_line_t *repl)
{
    /* mov ax, LABEL */
    if (!op_args(&w[0], "mov", 2)) return 0;
    if (strcmp(w[0].args[0], "ax")) return 0;
    const char *label = w[0].args[1];
    if (label[0] == '[') return 0; /* already indirect */

    /* mov bx, ax  or  mov si, ax  etc */
    if (!op_args(&w[1], "mov", 2)) return 0;
    if (strcmp(w[1].args[1], "ax")) return 0;
    char basereg[8];
    strcpy(basereg, w[1].args[0]);

    /* deref: mov ax, [basereg]  or mov ax, word [basereg] */
    if (!op_is(&w[2], "mov")) return 0;
    if (strcmp(w[2].args[0], "ax")) return 0;
    /* check arg1 matches [basereg] */
    {
        char expected[32];
        snprintf(expected, sizeof(expected), "[%s]", basereg);
        if (strcmp(w[2].args[1], expected)) return 0;
    }

    int n = 0;
    emit_repl(repl, &n, "\tmov ax, [%s]", label);
    return n;
}

/* 8086 Pattern 2: lea ax, [bp-N]; mov bx, ax; mov ax, [bx]  →  mov ax, [bp-N] */
static int i86_match_lea_deref(peep_line_t *w, peep_line_t *repl)
{
    /* lea ax, [bp-N] */
    if (!op_args(&w[0], "lea", 2)) return 0;
    if (strcmp(w[0].args[0], "ax")) return 0;
    const char *bp_expr = w[0].args[1];
    if (strncmp(bp_expr, "[bp", 3)) return 0;

    /* mov bx, ax */
    if (!op_args(&w[1], "mov", 2)) return 0;
    if (strcmp(w[1].args[1], "ax")) return 0;
    char basereg[8];
    strcpy(basereg, w[1].args[0]);

    /* mov ax, [basereg] */
    if (!op_args(&w[2], "mov", 2)) return 0;
    if (strcmp(w[2].args[0], "ax")) return 0;
    {
        char expected[32];
        snprintf(expected, sizeof(expected), "[%s]", basereg);
        if (strcmp(w[2].args[1], expected)) return 0;
    }

    int n = 0;
    emit_repl(repl, &n, "\tmov ax, %s", bp_expr);
    return n;
}

/* 8086 Pattern 3: lea ax, [bp-N]; push ax; mov ax, IMM; pop bx; mov [bx], ax
 * →  mov word [bp-N], IMM  (only when value is an immediate constant)
 */
static int i86_match_lea_push_store(peep_line_t *w, peep_line_t *repl)
{
    /* lea ax, [bp-N] */
    if (!op_args(&w[0], "lea", 2)) return 0;
    if (strcmp(w[0].args[0], "ax")) return 0;
    const char *bp_expr = w[0].args[1];
    if (strncmp(bp_expr, "[bp", 3)) return 0;

    /* push ax */
    if (!op_args(&w[1], "push", 1)) return 0;
    if (strcmp(w[1].args[0], "ax")) return 0;

    /* mov ax, IMM — must be an immediate */
    if (!op_args(&w[2], "mov", 2)) return 0;
    if (strcmp(w[2].args[0], "ax")) return 0;
    const char *val = w[2].args[1];
    {
        int dummy;
        if (!parse_arg_int(val, &dummy)) return 0; /* not an immediate */
    }

    /* pop REG */
    if (!op_args(&w[3], "pop", 1)) return 0;
    const char *popreg = w[3].args[0];

    /* mov [REG], ax */
    if (!op_args(&w[4], "mov", 2)) return 0;
    {
        char expected[32];
        snprintf(expected, sizeof(expected), "[%s]", popreg);
        if (strcmp(w[4].args[0], expected)) return 0;
    }
    if (strcmp(w[4].args[1], "ax")) return 0;

    int n = 0;
    emit_repl(repl, &n, "\tmov word %s, %s", bp_expr, val);
    return n;
}

/* 8086 Pattern 4: push ax; pop bx  →  mov bx, ax */
static int i86_match_push_pop(peep_line_t *w, peep_line_t *repl)
{
    if (!op_args(&w[0], "push", 1)) return 0;
    if (!op_args(&w[1], "pop", 1)) return 0;
    if (strcmp(w[0].args[0], w[1].args[0]) == 0) return 0; /* push ax; pop ax = no-op? No, it affects stack. */
    int n = 0;
    emit_repl(repl, &n, "\tmov %s, %s", w[1].args[0], w[0].args[0]);
    return n;
}

/* 8086 Pattern 5: mov bx, ax; mov [bx], ax  →  mov [bx-...], ax
 * This is only valid if bx is NEVER used again in the nearby code.
 * Too complex for peephole. Skip.
 */

/* 8086 Pattern 6: xchg ax, bx; sub ax, bx  →  sub bx, ax  (if we later want ax as result)
 * Not always valid. Skip.
 */

/* 8086 Pattern 7: mov ax, LABEL; push ax; mov ax, IMM; pop bx; mov [bx], ax
 * →  mov word [LABEL], IMM  (only when value is an immediate constant)
 */
static int i86_match_label_push_store(peep_line_t *w, peep_line_t *repl)
{
    /* mov ax, LABEL */
    if (!op_args(&w[0], "mov", 2)) return 0;
    if (strcmp(w[0].args[0], "ax")) return 0;
    const char *label = w[0].args[1];
    if (label[0] == '[') return 0;

    /* push ax */
    if (!op_args(&w[1], "push", 1)) return 0;
    if (strcmp(w[1].args[0], "ax")) return 0;

    /* mov ax, IMM */
    if (!op_args(&w[2], "mov", 2)) return 0;
    if (strcmp(w[2].args[0], "ax")) return 0;
    const char *val = w[2].args[1];
    {
        int dummy;
        if (!parse_arg_int(val, &dummy)) return 0; /* not an immediate */
    }

    /* pop REG */
    if (!op_args(&w[3], "pop", 1)) return 0;
    const char *popreg = w[3].args[0];

    /* mov [REG], ax */
    if (!op_args(&w[4], "mov", 2)) return 0;
    {
        char expected[32];
        snprintf(expected, sizeof(expected), "[%s]", popreg);
        if (strcmp(w[4].args[0], expected)) return 0;
    }
    if (strcmp(w[4].args[1], "ax")) return 0;

    int n = 0;
    emit_repl(repl, &n, "\tmov word [%s], %s", label, val);
    return n;
}

/* ── Apply 8086 patterns ────────────────────────────────────── */

static int i86_replace(peep_line_t *window, int wcount, peep_line_t *repl)
{
    int n;
    if (wcount == 2) {
        n = i86_match_push_pop(window, repl);
        if (n > 0) return n;
    }
    if (wcount == 3) {
        n = i86_match_label_deref(window, repl);
        if (n > 0) return n;
        n = i86_match_lea_deref(window, repl);
        if (n > 0) return n;
    }
    if (wcount == 5) {
        n = i86_match_lea_push_store(window, repl);
        if (n > 0) return n;
        n = i86_match_label_push_store(window, repl);
        if (n > 0) return n;
    }
    return 0;
}

/* ───────────────────────────────────────────────────────────────
 * 8080 / 8085 Peephole Patterns
 * ─────────────────────────────────────────────────────────────── */

/* 8080 Pattern 1: push h; pop d  →  xchg  (but xchg swaps DE with HL!)
 * push h; pop d preserves DE on stack, sets DE=HL. xchg swaps. NOT equivalent.
 *
 * 8080 Pattern 2: xchg; mov a, e; mov m, a; inx h; mov a, d; mov m, a
 * This stores DE to [HL]. Can't easily shorten on 8080.
 *
 * 8080 Pattern 3: dad b + local load
 * lxi h, -N; dad b  →  this computes BC-N in HL. Can't shorten.
 *
 * 8080 Pattern 4: push h; ... ; pop h with no changes → redundant. Hard to detect.
 *
 * Most 8080 patterns are limited due to the CPU's minimal instruction set.
 */

static int i80_replace(peep_line_t *window, int wcount, peep_line_t *repl)
{
    (void)window; (void)wcount; (void)repl;
    /* 8080 has few peephole optimizations */
    return 0;
}

/* ───────────────────────────────────────────────────────────────
 * 8086exe Peephole Patterns (32-bit values, far pointers)
 * ─────────────────────────────────────────────────────────────── */

/* 8086exe Pattern 1: push dx; push ax; pop bx; pop cx  →  mov cx, dx; mov bx, ax
 * (replaces 32-bit value push/pop pair with register moves)
 */
static int i86e_match_push32_pop32(peep_line_t *w, peep_line_t *repl)
{
    if (!op_args(&w[0], "push", 1)) return 0;
    if (!op_args(&w[1], "push", 1)) return 0;
    if (!op_args(&w[2], "pop", 1)) return 0;
    if (!op_args(&w[3], "pop", 1)) return 0;
    /* Don't optimize if pushing to same regs (identity) */
    if (!strcmp(w[0].args[0], w[3].args[0]) && !strcmp(w[1].args[0], w[2].args[0]))
        return 0;

    int n = 0;
    /* pop order: first pop goes to first pushed? No, LIFO:
     * push A; push B; pop C; pop D → C=B, D=A
     * So: mov D, A; mov C, B */
    emit_repl(repl, &n, "\tmov %s, %s", w[3].args[0], w[0].args[0]);
    emit_repl(repl, &n, "\tmov %s, %s", w[2].args[0], w[1].args[0]);
    return n;
}

/* 8086exe Pattern 2: lea ax, [bp+N]; mov bx, ax; mov ax, [bx]  →  mov ax, [bp+N]
 * (same as 8086 pattern but for far code)
 */
static int i86e_match_lea_deref(peep_line_t *w, peep_line_t *repl)
{
    /* Same logic as i86_match_lea_deref */
    if (!op_args(&w[0], "lea", 2)) return 0;
    if (strcmp(w[0].args[0], "ax")) return 0;
    const char *bp_expr = w[0].args[1];
    if (strncmp(bp_expr, "[bp", 3)) return 0;

    if (!op_args(&w[1], "mov", 2)) return 0;
    if (strcmp(w[1].args[1], "ax")) return 0;
    char basereg[8];
    strcpy(basereg, w[1].args[0]);

    if (!op_is(&w[2], "mov")) return 0;
    if (strcmp(w[2].args[0], "ax")) return 0;
    {
        char expected[32];
        snprintf(expected, sizeof(expected), "[%s]", basereg);
        if (strcmp(w[2].args[1], expected)) return 0;
    }

    int n = 0;
    emit_repl(repl, &n, "\tmov ax, %s", bp_expr);
    return n;
}

/* 8086exe Pattern 3: push dx; push ax followed by immediate pop into same regs
 * This is already handled by push32_pop32. No need for another.
 */

static int i86e_replace(peep_line_t *window, int wcount, peep_line_t *repl)
{
    int n;
    if (wcount == 3) {
        n = i86e_match_lea_deref(window, repl);
        if (n > 0) return n;
    }
    if (wcount == 4) {
        n = i86e_match_push32_pop32(window, repl);
        if (n > 0) return n;
    }
    return 0;
}

/* ───────────────────────────────────────────────────────────────
 * Common patterns (all targets)
 * ─────────────────────────────────────────────────────────────── */

/* After call cleanup: ex de,hl; ld hl,2; add hl,sp; ld sp,hl; ex de,hl
 * For Z80, this saves HL, pops 1 arg (2 bytes) from stack, restores HL.
 * On Z80 there's no add sp, imm. This pattern is already optimal.
 * For other targets (8086: add sp, N), it's already optimal.
 */

/* ───────────────────────────────────────────────────────────────
 * Main peephole engine
 * ─────────────────────────────────────────────────────────────── */

/* Check if a line is a section change / label that blocks optimization */
static int is_barrier(const peep_line_t *pl)
{
    if (!pl->is_inst) return 1;
    /* call, jmp, ret, jz, jnz etc — don't optimize across control flow */
    if (!strcmp(pl->opcode, "call")) return 1;
    if (!strcmp(pl->opcode, "ret")) return 1;
    if (!strcmp(pl->opcode, "jmp")) return 1;
    if (!strncmp(pl->opcode, "j", 1) && strlen(pl->opcode) <= 3) return 1; /* jz, jnz, jp, jc, etc */
    return 0;
}

void peep_apply(FILE *in, FILE *out, const char *target)
{
    peep_seq_t seq;
    seq_init(&seq);

    /* Read all lines */
    char linebuf[PEEP_MAX_LINE];
    while (fgets(linebuf, sizeof(linebuf), in)) {
        seq_add(&seq, linebuf);
    }

    /* Apply patterns with sliding window */
    peep_line_t repl[PEEP_WINDOW];
    int i = 0;
    while (i < seq.count) {
        /* Output non-instruction lines as-is */
        if (!seq.lines[i].is_inst) {
            fprintf(out, "%s", seq.lines[i].raw);
            int len = strlen(seq.lines[i].raw);
            if (len == 0 || seq.lines[i].raw[len-1] != '\n')
                fprintf(out, "\n");
            i++;
            continue;
        }

        /* Try patterns with decreasing window sizes */
        int matched = 0;
        int max_w = PEEP_WINDOW;
        if (i + max_w > seq.count) max_w = seq.count - i;

        for (int w = max_w; w >= 2; w--) {
            /* Check for barrier or non-inst inside window */
            int ok = 1;
            for (int k = 1; k < w; k++) {
                if (!seq.lines[i + k].is_inst || is_barrier(&seq.lines[i + k])) {
                    ok = 0; break;
                }
            }
            if (!ok) continue;

            int n = 0;

            if (!strcmp(target, "z80")) {
                n = z80_replace(&seq.lines[i], w, repl);
            } else if (!strcmp(target, "8086")) {
                n = i86_replace(&seq.lines[i], w, repl);
            } else if (!strcmp(target, "8080") || !strcmp(target, "8085")) {
                n = i80_replace(&seq.lines[i], w, repl);
            } else if (!strcmp(target, "8086exe") || !strcmp(target, "8086mz")) {
                n = i86e_replace(&seq.lines[i], w, repl);
            }

            if (n > 0 && n < w) {
                /* Replace w lines with n lines */
                for (int r = 0; r < n; r++) {
                    fprintf(out, "%s\n", repl[r].raw);
                }
                i += w;
                matched = 1;
                break;
            }
        }

        if (!matched) {
            /* No pattern matched, emit this line as-is */
            fprintf(out, "%s", seq.lines[i].raw);
            int len = strlen(seq.lines[i].raw);
            if (len == 0 || seq.lines[i].raw[len-1] != '\n')
                fprintf(out, "\n");
            i++;
        }
    }

    seq_free(&seq);
}
