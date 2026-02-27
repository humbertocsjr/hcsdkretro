#include "link.h"

void verbose(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if(_verbose) vprintf(fmt, args);
    va_end(args);
}