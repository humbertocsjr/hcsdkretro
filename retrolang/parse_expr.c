#include "retrolang.h"

expr_t *convert_expr(token_t *token)
{
    expr_t *obj = malloc(sizeof(expr_t) + strlen(token->token));
    if(!obj) error_token(token, "Expression memory overflow.");
    memset(obj, 0, sizeof(expr_t));
    obj->tok = token->tok;
    obj->line = token->line;
    obj->column = token->column;
    obj->source = token->source;
    obj->value = token->value;
    strcpy(obj->token, token->token);
    return obj;
}

expr_t *parse_expr0(source_t *src, command_t *cmd, func_t *func)
{
    expr_t *e = NULL;
    if(token_is(src, TOK_INTEGER) || token_is(src, TOK_STRING))
    {
        e = convert_expr(token_curr(src));
        token_scan(src);
    }
    else if(token_is(src, TOK_SYMBOL))
    {
        e = convert_expr(token_curr(src));
        token_scan(src);
        if(token_is(src, TOK_INDEX_OPEN))
        {
            e->tok = ACT_INDEXED;
            e->left = parse_expr(src, cmd, func);
        }
        else if(token_is(src, TOK_COMMA))
        {
            e->tok = ACT_CALL;
            e->right = parse_expr(src, cmd, func);
        }
    }
    else if(token_is(src, TOK_INDEX_OPEN))
    {
        e = convert_expr(token_curr(src));
        token_scan(src);
        e->left = parse_expr(src, cmd, func);
        match_token(token_is(src, TOK_INDEX_CLOSE), token_curr(src), "']' expected.");
        token_scan(src);
        if(token_is(src, TOK_INDEX_OPEN))
        {
            e->tok = ACT_INDEXED;
            e->right = parse_expr(src, cmd, func);
        }
        else if(token_is(src, TOK_COMMA))
        {
            e->tok = ACT_INDEXED_CALL;
            e->right = parse_expr(src, cmd, func);
        }
    }
    else if(token_is(src, TOK_PARAMS_OPEN))
    {
        token_scan(src);
        e = parse_expr(src, cmd, func);
        match_token(token_is(src, TOK_PARAMS_CLOSE), token_curr(src), "')' expected.");
        token_scan(src);
    }
    else error_token(token_curr(src), "expression expected.");
    return e;
}

expr_t *parse_expr1(source_t *src, command_t *cmd, func_t *func)
{
    expr_t *e = parse_expr0(src, cmd, func);
    while
    (
        token_is(src, TOK_SHIFT_LEFT) || 
        token_is(src, TOK_SHIFT_RIGHT)
    )
    {
        expr_t *op = convert_expr(token_curr(src));
        token_scan(src);
        op->left = e;
        op->right = parse_expr0(src, cmd, func);;
        e = op;
    }
    return e;
}

expr_t *parse_expr2(source_t *src, command_t *cmd, func_t *func)
{
    expr_t *e = parse_expr1(src, cmd, func);
    while(token_is(src, TOK_MUL) || token_is(src, TOK_DIV) || token_is(src, TOK_MOD) || token_is(src, TOK_AND))
    {
        expr_t *op = convert_expr(token_curr(src));
        token_scan(src);
        op->left = e;
        op->right = parse_expr1(src, cmd, func);;
        e = op;
    }
    return e;
}

expr_t *parse_expr3(source_t *src, command_t *cmd, func_t *func)
{
    expr_t *e = parse_expr2(src, cmd, func);
    while(token_is(src, TOK_ADD) || token_is(src, TOK_SUB) || token_is(src, TOK_OR) || token_is(src, TOK_XOR))
    {
        expr_t *op = convert_expr(token_curr(src));
        token_scan(src);
        op->left = e;
        op->right = parse_expr2(src, cmd, func);;
        e = op;
    }
    return e;
}

expr_t *parse_expr4(source_t *src, command_t *cmd, func_t *func)
{
    expr_t *e = parse_expr3(src, cmd, func);
    while
    (
        token_is(src, TOK_LESS_OR_EQUAL) || 
        token_is(src, TOK_LESS_THAN) || 
        token_is(src, TOK_GREATER_OR_EQUAL) || 
        token_is(src, TOK_GREATER_THAN) || 
        token_is(src, TOK_EQUAL) || 
        token_is(src, TOK_NOT_EQUAL)
    )
    {
        expr_t *op = convert_expr(token_curr(src));
        token_scan(src);
        op->left = e;
        op->right = parse_expr3(src, cmd, func);;
        e = op;
    }
    return e;
}

expr_t *parse_expr5(source_t *src, command_t *cmd, func_t *func)
{
    expr_t *e = parse_expr4(src, cmd, func);
    while(token_is(src, TOK_AND_ALSO))
    {
        expr_t *op = convert_expr(token_curr(src));
        token_scan(src);
        op->left = e;
        op->right = parse_expr4(src, cmd, func);;
        e = op;
    }
    return e;
}

expr_t *parse_expr6(source_t *src, command_t *cmd, func_t *func)
{
    expr_t *e = parse_expr5(src, cmd, func);
    while(token_is(src, TOK_OR_ELSE))
    {
        expr_t *op = convert_expr(token_curr(src));
        token_scan(src);
        op->left = e;
        op->right = parse_expr5(src, cmd, func);;
        e = op;
    }
    return e;
}

expr_t *parse_expr7(source_t *src, command_t *cmd, func_t *func)
{
    expr_t *e = parse_expr6(src, cmd, func);
    while
    (
        token_is(src, TOK_SUB_ASSIGN) || 
        token_is(src, TOK_ADD_ASSIGN) || 
        token_is(src, TOK_DIV_ASSIGN) || 
        token_is(src, TOK_MOD_ASSIGN) || 
        token_is(src, TOK_MUL_ASSIGN) || 
        token_is(src, TOK_SHIFT_LEFT_ASSIGN) || 
        token_is(src, TOK_SHIFT_RIGHT_ASSIGN)
    )
    {
        expr_t *op = convert_expr(token_curr(src));
        token_scan(src);
        op->left = e;
        op->right = parse_expr6(src, cmd, func);;
        e = op;
    }
    return e;
}

expr_t *parse_expr8(source_t *src, command_t *cmd, func_t *func)
{
    expr_t *e = parse_expr7(src, cmd, func);
    while
    (
        token_is(src, TOK_ASSIGN)
    )
    {
        expr_t *op = convert_expr(token_curr(src));
        token_scan(src);
        op->left = e;
        op->right = parse_expr7(src, cmd, func);;
        e = op;
    }
    return e;
}

expr_t *parse_expr9(source_t *src, command_t *cmd, func_t *func)
{
    expr_t *e = parse_expr8(src, cmd, func);
    while
    (
        token_is(src, TOK_COMMA)
    )
    {
        expr_t *op = convert_expr(token_curr(src));
        token_scan(src);
        op->left = e;
        op->right = parse_expr8(src, cmd, func);;
        e = op;
    }
    return e;
}

expr_t *optimize(source_t *src, command_t *cmd, func_t *func, expr_t *e)
{
    if(!e) return e;
    e->left = optimize(src, cmd, func, e->left);
    e->right = optimize(src, cmd, func, e->right);
    if(e->left && e->right && e->left->tok == TOK_INTEGER && e->right->tok == TOK_INTEGER) switch(e->tok)
    {
        case TOK_ADD:
            e->tok = TOK_INTEGER;
            e->value = e->left->value + e->right->value;
            break;
        case TOK_SUB:
            e->tok = TOK_INTEGER;
            e->value = e->left->value - e->right->value;
            break;
        case TOK_MUL:
            e->tok = TOK_INTEGER;
            e->value = e->left->value * e->right->value;
            break;
        case TOK_DIV:
            e->tok = TOK_INTEGER;
            e->value = e->left->value / e->right->value;
            break;
        case TOK_MOD:
            e->tok = TOK_INTEGER;
            e->value = e->left->value % e->right->value;
            break;
        case TOK_AND:
            e->tok = TOK_INTEGER;
            e->value = e->left->value & e->right->value;
            break;
        case TOK_OR:
            e->tok = TOK_INTEGER;
            e->value = e->left->value | e->right->value;
            break;
        case TOK_XOR:
            e->tok = TOK_INTEGER;
            e->value = e->left->value ^ e->right->value;
            break;
        case TOK_SHIFT_LEFT:
            e->tok = TOK_INTEGER;
            e->value = e->left->value << e->right->value;
            break;
        case TOK_SHIFT_RIGHT:
            e->tok = TOK_INTEGER;
            e->value = e->left->value >> e->right->value;
            break;
        default: break;
    }
    if(e->tok == TOK_INTEGER && e->left)
    {
        free(e->left);
        e->left = NULL;
    }
    if(e->tok == TOK_INTEGER && e->right)
    {
        free(e->right);
        e->right = NULL;
    }
    return e;
}

expr_t *parse_expr(source_t *src, command_t *cmd, func_t *func)
{
    expr_t *e = optimize(src, cmd, func, parse_expr9(src, cmd, func));
    if(token_is(src, TOK_SINGLE_EQUAL)) error_token(token_curr(src), "'=' operation is invalid.");
    return e;
}