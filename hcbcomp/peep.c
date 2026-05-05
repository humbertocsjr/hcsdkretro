#include "peep.h"
#include "bcomp.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

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
int peep_parse_line(peep_line_t *pl, const char *raw)
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
int peep_parse_arg_int(const char *arg, int *val)
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

// [English] Structure representing a sequence (buffer) of parsed assembly lines
// [Portuguese] Estrutura que representa uma sequência (buffer) de linhas de assembly analisadas
typedef struct {
    peep_line_t *lines;
    int count;
    int cap;
} peep_seq_t;

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
    peep_parse_line(&seq->lines[seq->count], raw);
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

// Helper: emits a replacement line / Auxiliar: emite uma linha de substituição

// [English] Formats and adds a replacement assembly line to the replacement buffer
// [Portuguese] Formata e adiciona uma linha de substituição de assembly ao buffer de substituição
void peep_emit_repl(peep_line_t *repl, int *pn, const char *fmt, ...)
{
    char buf[PEEP_MAX_LINE];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    peep_parse_line(&repl[*pn], buf);
    (*pn)++;
}

// Pattern matching helpers / Auxiliares de correspondência de padrões

// [English] Checks if a parsed line is an instruction with the given opcode
// [Portuguese] Verifica se uma linha analisada é uma instrução com o opcode fornecido
int peep_op_is(const peep_line_t *pl, const char *op)
{
    return pl->is_inst && !strcmp(pl->opcode, op);
}

// [English] Checks if a specific argument of a parsed line matches the given value
// [Portuguese] Verifica se um argumento específico de uma linha analisada corresponde ao valor fornecido
int peep_arg_is(const peep_line_t *pl, int idx, const char *val)
{
    if (idx >= pl->nargs) return 0;
    return !strcmp(pl->args[idx], val);
}

// [English] Checks if a line has the given opcode and number of arguments
// [Portuguese] Verifica se uma linha tem o opcode e número de argumentos fornecidos
int peep_op_args(const peep_line_t *pl, const char *op, int n)
{
    return peep_op_is(pl, op) && pl->nargs == n;
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
// applies target-specific peephole patterns via gen_peep_replace() callback,
// and writes the optimized assembly to the output file.
// [Portuguese] Ponto de entrada principal do otimizador peephole: lê todas as linhas
// do arquivo de entrada, aplica padrões peephole específicos do alvo via callback
// gen_peep_replace(), e escreve o assembly otimizado no arquivo de saída.
void peep_apply(FILE *in, FILE *out)
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

            // Call CPU-specific pattern matcher via backend hook
            int n = gen_peep_replace(&seq.lines[i], w, repl);

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
