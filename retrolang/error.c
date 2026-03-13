#include "retrolang.h"


void error(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    if(_out)
    {
        fclose(_out);
        remove(_out_name);
    }
    exit(1);
}

void error_token(token_t *token, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "error: ");
    fprintf(stderr, "%s:%i:%i: ", token->source->name, token->line, token->column);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    if(_out)
    {
        fclose(_out);
        remove(_out_name);
    }
    exit(1);
}

void match_token(bool cmp, token_t *token, char *fmt, ...)
{
    if(cmp) return;
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "error: ");
    fprintf(stderr, "%s:%i:%i: ", token->source->name, token->line, token->column);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    if(_out)
    {
        fclose(_out);
        remove(_out_name);
    }
    exit(1);
}


void error_expr(expr_t *e, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "error: ");
    fprintf(stderr, "%s:%i:%i: ", e->source->name, e->line, e->column);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    if(_out)
    {
        fclose(_out);
        remove(_out_name);
    }
    exit(1);
}