#include "peep.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

// Instruction representation / Representação de instrução
#define PEEP_MAX_LINE 512
#define PEEP_MAX_ARGS 16
#define PEEP_WINDOW   12

// [English] Structure representing a single parsed assembly line
// [Portuguese] Estrutura que representa uma única linha de assembly analisada
typedef struct {
    char raw[PEEP_MAX_LINE];
    const char *label;
    const char *comment;
    char opcode[64];
    char args[PEEP_MAX_ARGS][64];
    int  nargs;
    int  is_inst;
} peep_line_t;

// [English] Structure representing a sequence (buffer) of parsed assembly lines
// [Portuguese] Estrutura que representa uma sequência (buffer) de linhas de assembly analisadas
typedef struct {
    peep_line_t *lines;
    int count;
    int cap;
} peep_seq_t;

// Line parser / Analisador de linha

// [English] Skips whitespace characters (space and tab) in a string
// [Portuguese] Pula caracteres de espaço em branco (espaço e tabulação) em uma string
static const char *skip_space(const char *p)
{
    while (*p == ' ' || *p == '\t') p++;
    return p;
}

// [English] Parses a raw assembly line into a peep_line_t structure.
// Identifies labels, comments, directives, and instructions with their opcode/arguments.
// Returns 1 if the line is an instruction, 0 otherwise.
// [Portuguese] Analisa uma linha bruta de assembly em uma estrutura peep_line_t.
// Identifica rótulos, comentários, diretivas e instruções com seu opcode/argumentos.
// Retorna 1 se a linha for uma instrução, 0 caso contrário.
static int parse_line(peep_line_t *pl, const char *raw)
{
    memset(pl, 0, sizeof(*pl));
    strncpy(pl->raw, raw, PEEP_MAX_LINE - 1);
    pl->raw[PEEP_MAX_LINE - 1] = 0;

    const char *p = raw;

    // Skip leading whitespace / Pula espaços iniciais
    if (*p == '\t') p++;
    else if (*p == ' ' && *(p+1) == ' ' && *(p+2) == ' ') p += 3;
    else { pl->is_inst = 0; return 0; }

    p = skip_space(p);

    // Label detection / Detecção de rótulo
    if (isalpha(*p) || *p == '_' || *p == '.') {
        const char *end = p;
        while (isalnum(*end) || *end == '_' || *end == '.') end++;
        if (*end == ':') {
            pl->label = pl->raw + (p - raw);
            pl->is_inst = 0;
            return 0;
        }
    }

    // Comment detection / Detecção de comentário
    if (*p == ';') {
        pl->comment = pl->raw + (p - raw);
        pl->is_inst = 0;
        return 0;
    }

    // Directive detection / Detecção de diretiva
    if (*p == '.') {
        pl->is_inst = 0;
        return 0;
    }

    // Must be an instruction / Deve ser uma instrução
    pl->is_inst = 1;

    // Parse opcode / Analisa opcode
    const char *start = p;
    while (*p && !isspace(*p) && *p != '\t') p++;
    int len = p - start;
    if (len >= 63) len = 63;
    memcpy(pl->opcode, start, len);
    pl->opcode[len] = 0;

    // Parse arguments / Analisa argumentos
    pl->nargs = 0;
    while (*p && pl->nargs < PEEP_MAX_ARGS) {
        p = skip_space(p);
        if (*p == 0 || *p == '\n' || *p == '\r') break;
        if (*p == ';') break;
        start = p;
        while (*p && *p != ',' && *p != ';' && *p != '\n' && *p != '\r') p++;
        len = p - start;
        while (len > 0 && (start[len-1] == ' ' || start[len-1] == '\t')) len--;
        if (len >= 63) len = 63;
        memcpy(pl->args[pl->nargs], start, len);
        pl->args[pl->nargs][len] = 0;
        pl->nargs++;
        if (*p == ',') p++;
    }

    return 1;
}

// [English] Extracts an integer constant from an argument string.
// Supports decimal and hexadecimal formats. Returns 1 on success, 0 on failure.
// [Portuguese] Extrai uma constante inteira de uma string de argumento.
// Suporta formatos decimal e hexadecimal. Retorna 1 em caso de sucesso, 0 em caso de falha.
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
    if (*arg) return 0;
    *val = neg * v;
    return 1;
}

// Sequence buffer / Buffer de sequência

// [English] Initializes the peephole sequence buffer with an initial capacity of 1024 lines
// [Portuguese] Inicializa o buffer de sequência do peephole com capacidade inicial de 1024 linhas
static void seq_init(peep_seq_t *seq)
{
    seq->cap = 1024;
    seq->lines = (peep_line_t *)malloc(seq->cap * sizeof(peep_line_t));
    seq->count = 0;
}

// [English] Adds a raw assembly line to the sequence buffer, auto-expanding capacity
// [Portuguese] Adiciona uma linha bruta de assembly ao buffer de sequência, com expansão automática de capacidade
static void seq_add(peep_seq_t *seq, const char *raw)
{
    if (seq->count >= seq->cap) {
        seq->cap *= 2;
        seq->lines = (peep_line_t *)realloc(seq->lines, seq->cap * sizeof(peep_line_t));
    }
    parse_line(&seq->lines[seq->count], raw);
    seq->count++;
}

// [English] Frees the memory used by the sequence buffer
// [Portuguese] Libera a memória usada pelo buffer de sequência
static void seq_free(peep_seq_t *seq)
{
    free(seq->lines);
    seq->lines = NULL;
    seq->count = 0;
    seq->cap = 0;
}

// Helper: emits a replacement line into the shifted buffer / Auxiliar: emite uma linha de substituição no buffer deslocado

// [English] Formats and adds a replacement assembly line to the replacement buffer
// [Portuguese] Formata e adiciona uma linha de substituição de assembly ao buffer de substituição
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

// Pattern matching helpers / Auxiliares de correspondência de padrões

// [English] Checks if a parsed line is an instruction with the given opcode
// [Portuguese] Verifica se uma linha analisada é uma instrução com o opcode fornecido
static int op_is(const peep_line_t *pl, const char *op)
{
    return pl->is_inst && !strcmp(pl->opcode, op);
}

// [English] Checks if a specific argument of a parsed line matches the given value
// [Portuguese] Verifica se um argumento específico de uma linha analisada corresponde ao valor fornecido
static int arg_is(const peep_line_t *pl, int idx, const char *val)
{
    if (idx >= pl->nargs) return 0;
    return !strcmp(pl->args[idx], val);
}

// [English] Checks if a line has the given opcode and number of arguments
// [Portuguese] Verifica se uma linha tem o opcode e número de argumentos fornecidos
static int op_args(const peep_line_t *pl, const char *op, int n)
{
    return op_is(pl, op) && pl->nargs == n;
}

// [English] Z80 Pattern 1: optimizes local variable load with computed offset into direct IX load.
// Replaces: ld hl,CONST; ex de,hl; push ix; pop hl; add hl,de; ld a,[hl]; inc hl; ld h,[hl]; ld l,a
// With: ld l,[ix+CONST]; ld h,[ix+CONST+1]
// [Portuguese] Padrão Z80 1: otimiza carga de variável local com deslocamento computado para carga direta por IX.
// Substitui: ld hl,CONST; ex de,hl; push ix; pop hl; add hl,de; ld a,[hl]; inc hl; ld h,[hl]; ld l,a
// Por: ld l,[ix+CONST]; ld h,[ix+CONST+1]
static int z80_match_local_load(peep_line_t *w, peep_line_t *repl)
{
    if (w[0].nargs != 2) return 0;
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

// [English] Z80 Pattern 2: alternate register order for local variable load.
// Same as pattern 1 but with registers e/d instead of a and ex de,hl at end.
// Replaces 9 instructions with 2 direct IX loads.
// [Portuguese] Padrão Z80 2: ordem alternativa de registradores para carga de variável local.
// Mesmo que o padrão 1 mas com registradores e/d em vez de a e ex de,hl no final.
// Substitui 9 instruções por 2 cargas diretas por IX.
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

// [English] Z80 Pattern 5: placeholder for redundant load detection (not implemented)
// [Portuguese] Padrão Z80 5: placeholder para detecção de carga redundante (não implementado)
static int z80_match_redundant_load(peep_line_t *w, peep_line_t *repl)
{
    return 0;
}

// [English] Z80 Pattern 5 (store immediate): optimizes storing an immediate value to a local
// variable with computed offset. Replaces 12 instructions with 3: ld hl,IMM; ld [ix+CONST],l; ld [ix+CONST+1],h
// [Portuguese] Padrão Z80 5 (armazenar imediato): otimiza o armazenamento de um valor imediato
// em uma variável local com deslocamento computado. Substitui 12 instruções por 3.
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

// [English] Applies Z80-specific peephole patterns to a window of instructions.
// Tests window sizes 9 (local load patterns) and 12 (local store immediate).
// [Portuguese] Aplica padrões peephole específicos do Z80 a uma janela de instruções.
// Testa tamanhos de janela 9 (padrões de carga local) e 12 (armazenar imediato local).
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

// 8086 Peephole Patterns / Padrões Peephole 8086

// [English] 8086 Pattern 1: collapses "mov ax,LABEL; mov bx,ax; mov ax,[bx]" into "mov ax,[LABEL]"
// [Portuguese] Padrão 8086 1: colapsa "mov ax,LABEL; mov bx,ax; mov ax,[bx]" em "mov ax,[LABEL]"
static int i86_match_label_deref(peep_line_t *w, peep_line_t *repl)
{
    if (!op_args(&w[0], "mov", 2)) return 0;
    if (strcmp(w[0].args[0], "ax")) return 0;
    const char *label = w[0].args[1];
    if (label[0] == '[') return 0;

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
    emit_repl(repl, &n, "\tmov ax, [%s]", label);
    return n;
}

// [English] 8086 Pattern 2: collapses "lea ax,[bp-N]; mov bx,ax; mov ax,[bx]" into "mov ax,[bp-N]"
// [Portuguese] Padrão 8086 2: colapsa "lea ax,[bp-N]; mov bx,ax; mov ax,[bx]" em "mov ax,[bp-N]"
static int i86_match_lea_deref(peep_line_t *w, peep_line_t *repl)
{
    if (!op_args(&w[0], "lea", 2)) return 0;
    if (strcmp(w[0].args[0], "ax")) return 0;
    const char *bp_expr = w[0].args[1];
    if (strncmp(bp_expr, "[bp", 3)) return 0;

    if (!op_args(&w[1], "mov", 2)) return 0;
    if (strcmp(w[1].args[1], "ax")) return 0;
    char basereg[8];
    strcpy(basereg, w[1].args[0]);

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

// [English] 8086 Pattern 3: collapses "lea ax,[bp-N]; push ax; mov ax,IMM; pop bx; mov [bx],ax"
// into "mov word [bp-N], IMM"
// [Portuguese] Padrão 8086 3: colapsa "lea ax,[bp-N]; push ax; mov ax,IMM; pop bx; mov [bx],ax"
// em "mov word [bp-N], IMM"
static int i86_match_lea_push_store(peep_line_t *w, peep_line_t *repl)
{
    if (!op_args(&w[0], "lea", 2)) return 0;
    if (strcmp(w[0].args[0], "ax")) return 0;
    const char *bp_expr = w[0].args[1];
    if (strncmp(bp_expr, "[bp", 3)) return 0;

    if (!op_args(&w[1], "push", 1)) return 0;
    if (strcmp(w[1].args[0], "ax")) return 0;

    if (!op_args(&w[2], "mov", 2)) return 0;
    if (strcmp(w[2].args[0], "ax")) return 0;
    const char *val = w[2].args[1];
    {
        int dummy;
        if (!parse_arg_int(val, &dummy)) return 0;
    }

    if (!op_args(&w[3], "pop", 1)) return 0;
    const char *popreg = w[3].args[0];

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

// [English] 8086 Pattern 4: replaces "push ax; pop bx" with "mov bx, ax"
// [Portuguese] Padrão 8086 4: substitui "push ax; pop bx" por "mov bx, ax"
static int i86_match_push_pop(peep_line_t *w, peep_line_t *repl)
{
    if (!op_args(&w[0], "push", 1)) return 0;
    if (!op_args(&w[1], "pop", 1)) return 0;
    if (strcmp(w[0].args[0], w[1].args[0]) == 0) return 0;
    int n = 0;
    emit_repl(repl, &n, "\tmov %s, %s", w[1].args[0], w[0].args[0]);
    return n;
}

// [English] 8086 Pattern 7: collapses "mov ax,LABEL; push ax; mov ax,IMM; pop bx; mov [bx],ax"
// into "mov word [LABEL], IMM"
// [Portuguese] Padrão 8086 7: colapsa "mov ax,LABEL; push ax; mov ax,IMM; pop bx; mov [bx],ax"
// em "mov word [LABEL], IMM"
static int i86_match_label_push_store(peep_line_t *w, peep_line_t *repl)
{
    if (!op_args(&w[0], "mov", 2)) return 0;
    if (strcmp(w[0].args[0], "ax")) return 0;
    const char *label = w[0].args[1];
    if (label[0] == '[') return 0;

    if (!op_args(&w[1], "push", 1)) return 0;
    if (strcmp(w[1].args[0], "ax")) return 0;

    if (!op_args(&w[2], "mov", 2)) return 0;
    if (strcmp(w[2].args[0], "ax")) return 0;
    const char *val = w[2].args[1];
    {
        int dummy;
        if (!parse_arg_int(val, &dummy)) return 0;
    }

    if (!op_args(&w[3], "pop", 1)) return 0;
    const char *popreg = w[3].args[0];

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

// [English] Applies 8086-specific peephole patterns to a window of instructions.
// Tests window sizes 2 (push/pop), 3 (deref patterns), and 5 (store patterns).
// [Portuguese] Aplica padrões peephole específicos do 8086 a uma janela de instruções.
// Testa tamanhos de janela 2 (push/pop), 3 (padrões deref) e 5 (padrões store).
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

// [English] 8080/8085 pattern matcher: no significant peephole optimizations
// are available due to the CPU's minimal instruction set
// [Portuguese] Correspondência de padrões 8080/8085: nenhuma otimização peephole
// significativa está disponível devido ao conjunto de instruções mínimo da CPU
static int i80_replace(peep_line_t *window, int wcount, peep_line_t *repl)
{
    (void)window; (void)wcount; (void)repl;
    return 0;
}

// 8086exe Peephole Patterns (32-bit values, far pointers) / Padrões Peephole 8086exe (valores 32-bit, ponteiros far)

// [English] 8086exe Pattern 1: replaces a 32-bit push/pop pair with register moves.
// "push dx; push ax; pop bx; pop cx" becomes "mov cx, dx; mov bx, ax"
// [Portuguese] Padrão 8086exe 1: substitui um par push/pop de 32 bits por movimentações de registradores.
// "push dx; push ax; pop bx; pop cx" vira "mov cx, dx; mov bx, ax"
static int i86e_match_push32_pop32(peep_line_t *w, peep_line_t *repl)
{
    if (!op_args(&w[0], "push", 1)) return 0;
    if (!op_args(&w[1], "push", 1)) return 0;
    if (!op_args(&w[2], "pop", 1)) return 0;
    if (!op_args(&w[3], "pop", 1)) return 0;
    if (!strcmp(w[0].args[0], w[3].args[0]) && !strcmp(w[1].args[0], w[2].args[0]))
        return 0;

    int n = 0;
    emit_repl(repl, &n, "\tmov %s, %s", w[3].args[0], w[0].args[0]);
    emit_repl(repl, &n, "\tmov %s, %s", w[2].args[0], w[1].args[0]);
    return n;
}

// [English] 8086exe Pattern 2: same as 8086 lea/deref pattern for far code model.
// "lea ax,[bp+N]; mov bx,ax; mov ax,[bx]" becomes "mov ax,[bp+N]"
// [Portuguese] Padrão 8086exe 2: mesmo que o padrão lea/deref 8086 para modelo de código far.
// "lea ax,[bp+N]; mov bx,ax; mov ax,[bx]" vira "mov ax,[bp+N]"
static int i86e_match_lea_deref(peep_line_t *w, peep_line_t *repl)
{
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

// [English] Applies 8086exe-specific peephole patterns (32-bit far code model).
// Tests window sizes 3 (lea deref) and 4 (push32/pop32).
// [Portuguese] Aplica padrões peephole específicos do 8086exe (modelo de código far 32-bit).
// Testa tamanhos de janela 3 (lea deref) e 4 (push32/pop32).
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

// [English] Checks if a parsed line acts as an optimization barrier.
// Non-instruction lines, calls, returns, and jumps block peephole optimization.
// [Portuguese] Verifica se uma linha analisada atua como uma barreira de otimização.
// Linhas não-instrução, chamadas, retornos e saltos bloqueiam a otimização peephole.
static int is_barrier(const peep_line_t *pl)
{
    if (!pl->is_inst) return 1;
    if (!strcmp(pl->opcode, "call")) return 1;
    if (!strcmp(pl->opcode, "ret")) return 1;
    if (!strcmp(pl->opcode, "jmp")) return 1;
    if (!strncmp(pl->opcode, "j", 1) && strlen(pl->opcode) <= 3) return 1;
    return 0;
}

// [English] Main peephole optimizer entry point: reads all lines from the input file,
// applies target-specific peephole patterns using a sliding window approach,
// and writes the optimized assembly to the output file.
// [Portuguese] Ponto de entrada principal do otimizador peephole: lê todas as linhas
// do arquivo de entrada, aplica padrões peephole específicos do alvo usando uma
// abordagem de janela deslizante e escreve o assembly otimizado no arquivo de saída.
void peep_apply(FILE *in, FILE *out, const char *target)
{
    peep_seq_t seq;
    seq_init(&seq);

    // Read all lines / Lê todas as linhas
    char linebuf[PEEP_MAX_LINE];
    while (fgets(linebuf, sizeof(linebuf), in)) {
        seq_add(&seq, linebuf);
    }

    // Apply patterns with sliding window / Aplica padrões com janela deslizante
    peep_line_t repl[PEEP_WINDOW];
    int i = 0;
    while (i < seq.count) {

        // Output non-instruction lines as-is / Gera linhas não-instrução como estão
        if (!seq.lines[i].is_inst) {
            fprintf(out, "%s", seq.lines[i].raw);
            int len = strlen(seq.lines[i].raw);
            if (len == 0 || seq.lines[i].raw[len-1] != '\n')
                fprintf(out, "\n");
            i++;
            continue;
        }

        // Try patterns with decreasing window sizes / Tenta padrões com tamanhos de janela decrescentes
        int matched = 0;
        int max_w = PEEP_WINDOW;
        if (i + max_w > seq.count) max_w = seq.count - i;

        for (int w = max_w; w >= 2; w--) {

            // Check for barrier or non-inst inside window / Verifica barreira ou não-instrução dentro da janela
            int ok = 1;
            for (int k = 1; k < w; k++) {
                if (!seq.lines[i + k].is_inst || is_barrier(&seq.lines[i + k])) {
                    ok = 0; break;
                }
            }
            if (!ok) continue;

            int n = 0;

            // Select target-specific pattern matcher / Seleciona o correspondente de padrões específico do alvo
            if (!strcmp(target, "z80")) {
                n = z80_replace(&seq.lines[i], w, repl);
            } else if (!strcmp(target, "8086")) {
                n = i86_replace(&seq.lines[i], w, repl);
            } else if (!strcmp(target, "8080") || !strcmp(target, "8085")) {
                n = i80_replace(&seq.lines[i], w, repl);
            } else if (!strcmp(target, "8086exe") || !strcmp(target, "8086mz")) {
                n = i86e_replace(&seq.lines[i], w, repl);
            }

            // If pattern matched, replace w lines with n lines / Se o padrão correspondeu, substitui w linhas por n linhas
            if (n > 0 && n < w) {
                for (int r = 0; r < n; r++) {
                    fprintf(out, "%s\n", repl[r].raw);
                }
                i += w;
                matched = 1;
                break;
            }
        }

        // No pattern matched, emit line as-is / Nenhum padrão correspondeu, emite linha como está
        if (!matched) {
            fprintf(out, "%s", seq.lines[i].raw);
            int len = strlen(seq.lines[i].raw);
            if (len == 0 || seq.lines[i].raw[len-1] != '\n')
                fprintf(out, "\n");
            i++;
        }
    }

    seq_free(&seq);
}
