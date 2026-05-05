#include "link.h"

// [English] Print an error message to stderr and exit with code 1
// [Portuguese] Imprime uma mensagem de erro no stderr e sai com código 1
void error(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}
