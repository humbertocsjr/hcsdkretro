#include "bcomp.h"

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
