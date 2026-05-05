#pragma once
#include <stdio.h>

// [English] Structure for a single parsed assembly line used by the peephole optimizer
// [Portuguese] Estrutura para uma única linha de assembly analisada usada pelo otimizador peephole
#define PEEP_MAX_LINE 512
#define PEEP_MAX_ARGS 16
#define PEEP_WINDOW   12

typedef struct {
    char raw[PEEP_MAX_LINE];
    const char *label;
    const char *comment;
    char opcode[64];
    char args[PEEP_MAX_ARGS][64];
    int  nargs;
    int  is_inst;
} peep_line_t;

// [English] Parses a raw assembly line into a peep_line_t structure.
// Returns 1 if the line is an instruction, 0 otherwise.
// [Portuguese] Analisa uma linha bruta de assembly em uma estrutura peep_line_t.
// Retorna 1 se a linha for uma instrução, 0 caso contrário.
int peep_parse_line(peep_line_t *pl, const char *raw);

// [English] Checks if a parsed line is an instruction with the given opcode
// [Portuguese] Verifica se uma linha analisada é uma instrução com o opcode fornecido
int peep_op_is(const peep_line_t *pl, const char *op);

// [English] Checks if a specific argument matches the given value
// [Portuguese] Verifica se um argumento específico corresponde ao valor fornecido
int peep_arg_is(const peep_line_t *pl, int idx, const char *val);

// [English] Checks if a line has the given opcode and number of arguments
// [Portuguese] Verifica se uma linha tem o opcode e número de argumentos fornecidos
int peep_op_args(const peep_line_t *pl, const char *op, int n);

// [English] Extracts an integer constant from an argument string (supports decimal/hex)
// [Portuguese] Extrai uma constante inteira de uma string de argumento (suporta decimal/hexa)
int peep_parse_arg_int(const char *arg, int *val);

// [English] Formats and adds a replacement assembly line to the replacement buffer
// [Portuguese] Formata e adiciona uma linha de substituição de assembly ao buffer de substituição
void peep_emit_repl(peep_line_t *repl, int *pn, const char *fmt, ...);

// [English] Main peephole optimizer entry point: reads from input, applies target-specific
// patterns via gen_peep_replace(), writes optimized assembly to output.
// [Portuguese] Ponto de entrada principal do otimizador peephole: lê da entrada, aplica
// padrões específicos do alvo via gen_peep_replace(), escreve assembly otimizado na saída.
void peep_apply(FILE *in, FILE *out);
