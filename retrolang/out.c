#include "retrolang.h"


void out_inline(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(_out, fmt, args);
    va_end(args);
}

void out_inline_vargs(char *fmt, va_list args)
{
    vfprintf(_out, fmt, args);
}

void out_line(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(_out, fmt, args);
    va_end(args);
    fprintf(_out, "\n");
}