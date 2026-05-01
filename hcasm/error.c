#include "asm.h"

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
