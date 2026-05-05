#pragma once
#include <stdio.h>

// [English] Applies peephole optimizations to the assembly output:
// reads from the input file stream and writes optimized assembly to the output stream
// [Portuguese] Aplica otimizações peephole na saída assembly:
// lê do fluxo de arquivo de entrada e escreve assembly otimizado no fluxo de saída
void peep_apply(FILE *in, FILE *out, const char *target);
