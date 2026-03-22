#include "retrolang.h"

#define TRYPARSE(key, val) if(!strcmp(key, token->token)) {token->tok = val;}

token_t *token_scan(source_t *src)
{
    memcpy(&src->curr_token, &src->next_token, sizeof(token_t));
    memset(&src->next_token, 0, sizeof(token_t));
    token_t *token = &src->next_token;
    while(source_is_space(src) || source_is_equal(src, ';'))
    {
        if(source_is_equal(src, ';'))
        {
            while(!source_is_equal(src, '\n') && !source_is_equal(src, 0))
            {
                source_next_char(src);
            }
        }
        else source_next_char(src);
    }
    token->tok = TOK_EOF;
    token->line = src->line;
    token->column = src->column;
    token->source = src;
    if(source_is_equal(src, 0))
    {
        return &src->curr_token;
    }
    else if(source_is_equal(src, '\n'))
    {
        token->tok = TOK_NEWLINE;
        while(source_is_space(src) || source_is_equal(src, '\n'))
        {
            source_next_char(src);
        }
    }
    else if(source_is_digit(src))
    {
        token->tok = TOK_INTEGER;
        if(source_is_equal(src, '0'))
        {
            source_concat_char(src);
            source_next_char(src);
            switch(source_get_char(src))
            {
                case 'x':
                case 'X':
                    source_concat_char(src);
                    source_next_char(src);
                    while(source_is_hexadecimal(src))
                    {
                        if(!source_is_equal(src, '_'))
                        {
                            token->value <<= 4;
                            if(source_is_between(src, 'a', 'f'))
                            {
                                token->value += source_get_char(src) - 'a' + 10;
                            }
                            else if(source_is_between(src, 'A', 'F'))
                            {
                                token->value += source_get_char(src) - 'A' + 10;
                            }
                            else
                            {
                                token->value += source_get_char(src) - '0';
                            }
                        }
                        source_concat_char(src);
                        source_next_char(src);
                    }
                    break;
                case 'o':
                case 'O':
                    source_concat_char(src);
                    source_next_char(src);
                    while(source_is_hexadecimal(src))
                    {
                        if(!source_is_equal(src, '_'))
                        {
                            token->value <<= 3;
                            token->value += source_get_char(src) - '0';
                        }
                        source_concat_char(src);
                        source_next_char(src);
                    }
                    break;
                case 'b':
                case 'B':
                    source_concat_char(src);
                    source_next_char(src);
                    while(source_is_binary(src))
                    {
                        if(!source_is_equal(src, '_'))
                        {
                            token->value <<= 1;
                            token->value += source_get_char(src) - '0';
                        }
                        source_concat_char(src);
                        source_next_char(src);
                    }
                    break;
                default:
                    while(source_is_digit(src))
                    {
                        if(!source_is_equal(src, '_'))
                        {
                            token->value *= 10;
                            token->value += source_get_char(src) - '0';
                        }
                        source_concat_char(src);
                        source_next_char(src);
                    }
                    break;
            }

        }
        else
        {
            while(source_is_digit(src))
            {
                if(!source_is_equal(src, '_'))
                {
                    token->value *= 10;
                    token->value += source_get_char(src) - '0';
                }
                source_concat_char(src);
                source_next_char(src);
            }
        }
    }
    else if(source_is_symbol(src))
    {
        token->tok = TOK_SYMBOL;
        while(source_is_symbol(src))
        {
            source_concat_lower_char(src);
            source_next_char(src);
        }
        TRYPARSE("def", KEY_DEF);
        TRYPARSE("if", KEY_IF);
        TRYPARSE("until", KEY_UNTIL);
        TRYPARSE("else", KEY_ELSE);
        TRYPARSE("while", KEY_WHILE);
        TRYPARSE("for", KEY_FOR);
        TRYPARSE("foreach", KEY_FOREACH);
        TRYPARSE("end", KEY_END);
        TRYPARSE("var", KEY_VAR);
        TRYPARSE("as", KEY_AS);
        TRYPARSE("typedef", KEY_TYPEDEF);
        TRYPARSE("asm", KEY_ASM);
        TRYPARSE("return", KEY_RETURN);
        TRYPARSE("decl", KEY_DECL);
        TRYPARSE("addressof", KEY_ADDRESSOF);
        TRYPARSE("sizeof", KEY_SIZEOF);
        TRYPARSE("include", KEY_INCLUDE);
    }
    else if(source_is_equal(src, '"'))
    {
        token->tok = TOK_STRING;
        source_next_char(src);
        while(!source_is_equal(src, '"'))
        {
            source_concat_escaped_char(src);
            source_next_char(src);
        }
        if(!source_is_equal(src, '"')) error_token(token, "'\"' espected. Found: '%c'", source_get_char(src));
        source_next_char(src);
    }
    else if(source_is_equal(src, '\''))
    {
        token->tok = TOK_INTEGER;
        source_next_char(src);
        while(!source_is_equal(src, '\''))
        {
            source_concat_escaped_char(src);
            token->value <<= 8;
            token->value += source_get_char(src);
            source_next_char(src);
        }
        if(!source_is_equal(src, '\'')) error_token(token, "''' espected. Found: '%c'", source_get_char(src));
        source_next_char(src);
    }
    else if(source_is_equal(src, '+'))
    {
        token->tok = TOK_ADD;
        source_concat_char(src);
        source_next_char(src);
        if(source_is_equal(src, '='))
        {
            token->tok = TOK_ADD_ASSIGN;
            source_concat_char(src);
            source_next_char(src);
        }
    }
    else if(source_is_equal(src, '-'))
    {
        token->tok = TOK_SUB;
        source_concat_char(src);
        source_next_char(src);
        if(source_is_equal(src, '='))
        {
            token->tok = TOK_SUB_ASSIGN;
            source_concat_char(src);
            source_next_char(src);
        }
    }
    else if(source_is_equal(src, '*'))
    {
        token->tok = TOK_MUL;
        source_concat_char(src);
        source_next_char(src);
        if(source_is_equal(src, '='))
        {
            token->tok = TOK_MUL_ASSIGN;
            source_concat_char(src);
            source_next_char(src);
        }
    }
    else if(source_is_equal(src, '/'))
    {
        token->tok = TOK_DIV;
        source_concat_char(src);
        source_next_char(src);
        if(source_is_equal(src, '='))
        {
            token->tok = TOK_DIV_ASSIGN;
            source_concat_char(src);
            source_next_char(src);
        }
    }
    else if(source_is_equal(src, ':'))
    {
        token->tok = TOK_INLINE_ELSE;
        source_concat_char(src);
        source_next_char(src);
        if(source_is_equal(src, '='))
        {
            token->tok = TOK_ASSIGN;
            source_concat_char(src);
            source_next_char(src);
        }
    }
    else if(source_is_equal(src, '='))
    {
        token->tok = TOK_SINGLE_EQUAL;
        source_concat_char(src);
        source_next_char(src);
        if(source_is_equal(src, '='))
        {
            token->tok = TOK_EQUAL;
            source_concat_char(src);
            source_next_char(src);
        }
    }
    else if(source_is_equal(src, '<'))
    {
        token->tok = TOK_LESS_THAN;
        source_concat_char(src);
        source_next_char(src);
        if(source_is_equal(src, '>'))
        {
            token->tok = TOK_NOT_EQUAL;
            source_concat_char(src);
            source_next_char(src);
        }
        else if(source_is_equal(src, '='))
        {
            token->tok = TOK_LESS_OR_EQUAL;
            source_concat_char(src);
            source_next_char(src);
        }
        else if(source_is_equal(src, '<'))
        {
            token->tok = TOK_SHIFT_LEFT;
            source_concat_char(src);
            source_next_char(src);
            if(source_is_equal(src, '='))
            {
                token->tok = TOK_SHIFT_LEFT_ASSIGN;
                source_concat_char(src);
                source_next_char(src);
            }
        }
    }
    else if(source_is_equal(src, '>'))
    {
        token->tok = TOK_GREATER_THAN;
        source_concat_char(src);
        source_next_char(src);
        if(source_is_equal(src, '='))
        {
            token->tok = TOK_GREATER_OR_EQUAL;
            source_concat_char(src);
            source_next_char(src);
        }
        else if(source_is_equal(src, '>'))
        {
            token->tok = TOK_SHIFT_RIGHT;
            source_concat_char(src);
            source_next_char(src);
            if(source_is_equal(src, '='))
            {
                token->tok = TOK_SHIFT_RIGHT_ASSIGN;
                source_concat_char(src);
                source_next_char(src);
            }
        }
    }
    else if(source_is_equal(src, '?'))
    {
        token->tok = TOK_INLINE_IF;
        source_concat_char(src);
        source_next_char(src);
    }
    else if(source_is_equal(src, '@'))
    {
        token->tok = TOK_POINTER;
        source_concat_char(src);
        source_next_char(src);
    }
    else if(source_is_equal(src, '('))
    {
        token->tok = TOK_PARAMS_OPEN;
        source_concat_char(src);
        source_next_char(src);
    }
    else if(source_is_equal(src, ')'))
    {
        token->tok = TOK_PARAMS_CLOSE;
        source_concat_char(src);
        source_next_char(src);
    }
    else if(source_is_equal(src, '['))
    {
        token->tok = TOK_INDEX_OPEN;
        source_concat_char(src);
        source_next_char(src);
    }
    else if(source_is_equal(src, ']'))
    {
        token->tok = TOK_INDEX_CLOSE;
        source_concat_char(src);
        source_next_char(src);
    }
    else if(source_is_equal(src, ','))
    {
        token->tok = TOK_COMMA;
        source_concat_char(src);
        source_next_char(src);
    }
    else if(source_is_equal(src, '%'))
    {
        token->tok = TOK_MOD;
        source_concat_char(src);
        source_next_char(src);
        if(source_is_equal(src, '='))
        {
            token->tok = TOK_MOD_ASSIGN;
            source_concat_char(src);
            source_next_char(src);
        }
    }
    else if(source_is_equal(src, '|'))
    {
        token->tok = TOK_OR;
        source_concat_char(src);
        source_next_char(src);
        if(source_is_equal(src, '|'))
        {
            token->tok = TOK_OR_ELSE;
            source_concat_char(src);
            source_next_char(src);
        }
    }
    else if(source_is_equal(src, '&'))
    {
        token->tok = TOK_AND;
        source_concat_char(src);
        source_next_char(src);
        if(source_is_equal(src, '&'))
        {
            token->tok = TOK_AND_ALSO;
            source_concat_char(src);
            source_next_char(src);
        }
    }
    else if(source_is_equal(src, '^'))
    {
        token->tok = TOK_XOR;
        source_concat_char(src);
        source_next_char(src);
        if(source_is_equal(src, '='))
        {
            token->tok = TOK_XOR_ASSIGN;
            source_concat_char(src);
            source_next_char(src);
        }
    }
    else error_token(token, "Unknown char: %c", source_get_char(src));
    return &src->curr_token;
}

token_t *token_curr(source_t *src)
{
    return &src->curr_token;
}

token_t *token_peek(source_t *src)
{
    return &src->next_token;
}

bool token_is(source_t *src, tok_t tok)
{
    return token_curr(src)->tok == tok;
}