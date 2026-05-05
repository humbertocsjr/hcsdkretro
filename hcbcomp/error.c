#include "bcomp.h"

// [English] Prints a formatted error message with the source file location
// (filename:line:column) and terminates the compiler with exit code 1
// [Portuguese] Imprime uma mensagem de erro formatada com a localização no
// arquivo fonte (arquivo:linha:coluna) e encerra o compilador com código de saída 1
void error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (filename)
    {
        fprintf(stderr, "%s:%i:%i ", filename, line_num, col_num);
    }
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}
