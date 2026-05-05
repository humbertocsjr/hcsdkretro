#include "asm.h"

// [English] Print error message with source position (if available) and exit
// [Portuguese] Imprime mensagem de erro com posição no fonte (se disponível) e sai
void error(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (_source)
    {
        fprintf(stderr, "%s:%i:%i ", _source->filename, _source->line, _source->column);
    }
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}

// [English] Print error with expression source position (file:line:col) and exit
// [Portuguese] Imprime erro com posição no fonte da expressão (arquivo:linha:coluna) e sai
void error_expr(expr_t *e, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (e)
    {
        fprintf(stderr, "%s:%i:%i ", e->filename, e->line, e->column);
    }
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}
