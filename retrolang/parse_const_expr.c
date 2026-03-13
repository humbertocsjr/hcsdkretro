#include "retrolang.h"

int32_t parse_const_expr0(source_t *src, command_t *cmd, func_t *func)
{
    int32_t value = 0;
    if(token_is(src, TOK_INTEGER))
    {
        value = token_curr(src)->value;
        token_scan(src);
    }
    return value;
}

int32_t parse_const_expr1(source_t *src, command_t *cmd, func_t *func)
{
    int32_t value = parse_const_expr0(src, cmd, func);
    while(token_is(src, TOK_SHIFT_LEFT) || token_is(src, TOK_SHIFT_RIGHT))
    {
        switch (token_curr(src)->tok)
        {
            case TOK_SHIFT_LEFT:
                token_scan(src);
                value <<= parse_const_expr0(src, cmd, func);
                break;
            case TOK_SHIFT_RIGHT:
                token_scan(src);
                value >>= parse_const_expr0(src, cmd, func);
                break;
            default: break;
        }
    }
    return value;
}

int32_t parse_const_expr2(source_t *src, command_t *cmd, func_t *func)
{
    int32_t value = parse_const_expr1(src, cmd, func);
    while(token_is(src, TOK_MUL) || token_is(src, TOK_DIV))
    {
        switch (token_curr(src)->tok)
        {
            case TOK_MUL:
                token_scan(src);
                value *= parse_const_expr1(src, cmd, func);
                break;
            case TOK_DIV:
                token_scan(src);
                int32_t div_value = parse_const_expr1(src, cmd, func);
                if(!div_value) error_token(token_curr(src), "division by zero.");
                value /= div_value;
                break;
            default: break;
        }
    }
    return value;
}

int32_t parse_const_expr3(source_t *src, command_t *cmd, func_t *func)
{
    int32_t value = parse_const_expr2(src, cmd, func);
    while(token_is(src, TOK_ADD) || token_is(src, TOK_SUB))
    {
        switch (token_curr(src)->tok)
        {
            case TOK_ADD:
                token_scan(src);
                value += parse_const_expr2(src, cmd, func);
                break;
            case TOK_SUB:
                token_scan(src);
                value -= parse_const_expr2(src, cmd, func);
                break;
            default: break;
        }
    }
    return value;
}


int32_t parse_const_expr(source_t *src, command_t *cmd, func_t *func)
{
    int32_t value = parse_const_expr3(src, cmd, func);
    return value;
}