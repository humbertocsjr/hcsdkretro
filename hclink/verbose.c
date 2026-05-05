#include "link.h"

// [English] Print a verbose/debug message if verbose mode is enabled
// [Portuguese] Imprime uma mensagem verbose/debug se o modo verbose estiver ativado
void verbose(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (_verbose)
        vprintf(fmt, args);
    va_end(args);
}
