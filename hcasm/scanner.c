#include "asm.h"

static expr_t *_current = NULL;
static expr_t *_next = NULL;

#define CAT() c[0] = source_getc(); strncat(token, c, 255);
#define CATNEXT() CAT(); source_nextc();

static inline int hex_val(char c)
{
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'a' && c <= 'f') return c - 'a' + 10;
    return c - 'A' + 10;
}
#define CATESC() c[0] = source_getescapec(); strncat(token, c, 255);
#define CATESCNEXT() CATESC(); source_nextc();

bool curr_is(token_t token)
{
    return _current->token == token;
}

bool curr_is_keyword(char *keyword)
{
    char *s1 = _current->text;
    char *s2 = keyword;
    while(tolower(*s1) == tolower(*s2))
    {
        if(*s1 == 0) return true;
        s1++;
        s2++;
    }
    return false;
}

bool is_keyword(char *token, char *keyword)
{
    char *s1 = token;
    char *s2 = keyword;
    while(tolower(*s1) == tolower(*s2))
    {
        if(*s1 == 0) return true;
        s1++;
        s2++;
    }
    return false;
}

bool next_is(token_t token)
{
    return _next->token == token;
}

expr_t *curr()
{
    if(!_current) error("current token not exists");
    return _current;
}

expr_t *clone_expr(expr_t *src)
{
    expr_t *e = malloc(sizeof(expr_t) + strlen(src->text));
    memcpy(e, src, sizeof(expr_t) + strlen(src->text));
    return e;
}

void free_expr(expr_t *e)
{
    if(!e) return;
    if(e->left) free_expr(e->left);
    if(e->right) free_expr(e->right);
    free(e);
}

expr_t *scan()
{
    char c[2];
    c[1] = 0;
    expr_t e;
    reg_t *reg;
    char token[256];
    strcpy(token, "");
    memset(&e, 0, sizeof(expr_t));
    if(_current == NULL)
    {
        free(_current);
    }
    if(_next != NULL)
    {
        _current = _next;
    }
    _next = NULL;
    while(source_is(' ') || source_is('\t') || source_is('\r') || source_is(';'))
    { 
        if(source_is(';'))
        {
            while(!source_is('\n') && !source_is(0)) source_nextc();
        }
        else source_nextc();
    }
    e.line = _source->line;
    e.column = _source->column;
    e.filename = _source->filename;
    if(source_is('0'))
    {
        e.token = TOK_VALUE;
        CATNEXT();
        if(source_is('b'))
        {
            CATNEXT();
            while(source_between('0', '1'))
            {
                e.value <<= 1;
                e.value += source_getc() - '0';
                CATNEXT();
            }
        }
        else if(source_is('o'))
        {
            CATNEXT();
            while(source_between('0', '7'))
            {
                e.value <<= 3;
                e.value += source_getc() - '0';
                CATNEXT();
            }
        }
        else if(source_is('x'))
        {
            CATNEXT();
            while(source_between('0', '9') || source_between('a', 'f') || source_between('A', 'F'))
            {
                e.value <<= 4;
                if(source_between('a', 'f'))
                    e.value += source_getc() - 'a' + 10;
                else if(source_between('A', 'F'))
                    e.value += source_getc() - 'A' + 10;
                else
                    e.value += source_getc() - '0';
                CATNEXT();
            }
        }
        else 
        {
            bool has_non_octal = false;
            int hex_count = 0;
            while(source_between('0', '9') || source_between('a', 'f') || source_between('A', 'F'))
            {
                if(!source_between('0', '7')) has_non_octal = true;
                CATNEXT();
                hex_count++;
            }
            if(source_is('h') || source_is('H'))
            {
                CATNEXT();
                char *p = token;
                e.value = 0;
                while(*p && *p != 'h' && *p != 'H')
                {
                    e.value = (e.value << 4) | hex_val(*p);
                    p++;
                }
            }
            else if(!has_non_octal && hex_count > 0)
            {
                e.value = 0;
                char *p = token;
                while(*p)
                {
                    e.value <<= 3;
                    e.value += *p - '0';
                    p++;
                }
            }
            else
            {
                strcpy(token, "0");
                e.value = 0;
            }
        }
    }
    else if(source_between('0', '9'))
    {
        e.token = TOK_VALUE;
        while(source_between('0', '9'))
        {
            e.value *= 10;
            e.value += source_getc() - '0';
            CATNEXT();
        }
        if(source_is('h') || source_is('H'))
        {
            CATNEXT();
            char *p = token;
            e.value = 0;
            while(*p && *p != 'h' && *p != 'H')
            {
                e.value = (e.value << 4) | hex_val(*p);
                p++;
            }
        }
    }
    else if(source_between('0', '9') || source_between('a', 'z') || source_between('A', 'Z') || source_is('_') || source_is('.'))
    {
        e.token = TOK_SYMBOL;
        while(source_between('0', '9') || source_between('a', 'z') || source_between('A', 'Z') || source_is('_') || source_is('.'))
        {
            CATNEXT();
        }
        if(source_is('\''))
        {
            CATNEXT();
        }
        reg = _regs;
        while(reg->name)
        {
            if(is_keyword(reg->name, token))
            {
                e.reg = reg;
                e.token = TOK_REGISTER;
                break;
            }
            reg++;
        }
        if(e.token == TOK_SYMBOL)
        {
            int tlen = strlen(token);
            if(tlen > 1 && (token[tlen-1] == 'h' || token[tlen-1] == 'H'))
            {
                bool is_hex = true;
                for(int i = 0; i < tlen-1; i++)
                {
                    char c = token[i];
                    if(!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
                    {
                        is_hex = false;
                        break;
                    }
                }
                if(is_hex)
                {
                    e.token = TOK_VALUE;
                    e.value = 0;
                    for(int i = 0; i < tlen-1; i++)
                        e.value = (e.value << 4) | hex_val(token[i]);
                }
            }
        }
        if(token[0] == '.')
        {
            if(_current_label)
            {
                char *label = malloc(strlen(token) + strlen(_current_label) + 1);
                strcpy(label, _current_label);
                strcat(label, token);
                strncpy(token, label, 255);
                free(label);
            }
            e.token = TOK_SUB_LABEL;
        }
    }
    else if(source_is('\''))
    {
        e.token = TOK_VALUE;
        source_nextc();
        while(!source_is('\''))
        {
            e.value <<= 8;
            e.value |= source_getescapec();
            CATNEXT();
        }
        if(!source_is('\'')) error("\"'\" expected.");
        source_nextc();
    }
    else if(source_is('"'))
    {
        e.token = TOK_STRING;
        source_nextc();
        while(!source_is('"'))
        {
            CATESCNEXT();
        }
        if(!source_is('"')) error("'\"' expected.");
        source_nextc();
    }
    else if(source_is('+'))
    {
        e.token = TOK_ADD;
        CATNEXT();
    }
    else if(source_is('-'))
    {
        e.token = TOK_SUB;
        CATNEXT();
    }
    else if(source_is('/'))
    {
        e.token = TOK_DIV;
        CATNEXT();
    }
    else if(source_is('%'))
    {
        e.token = TOK_MOD;
        CATNEXT();
    }
    else if(source_is('*'))
    {
        e.token = TOK_MUL;
        CATNEXT();
    }
    else if(source_is('#'))
    {
        e.token = TOK_HASH;
        CATNEXT();
    }
    else if(source_is(':'))
    {
        e.token = TOK_COLON;
        CATNEXT();
    }
    else if(source_is('$'))
    {
        e.token = TOK_CURRENT_POS;
        CATNEXT();
    }
    else if(source_is(','))
    {
        e.token = TOK_COMMA;
        CATNEXT();
    }
    else if(source_is('('))
    {
        e.token = TOK_PARAMS_OPEN;
        CATNEXT();
    }
    else if(source_is(')'))
    {
        e.token = TOK_PARAMS_CLOSE;
        CATNEXT();
    }
    else if(source_is('['))
    {
        e.token = TOK_INDEX_OPEN;
        CATNEXT();
    }
    else if(source_is(']'))
    {
        e.token = TOK_INDEX_CLOSE;
        CATNEXT();
    }
    else if(source_is('\n'))
    {
        e.token = TOK_NEWLINE;
        CATNEXT();
    }
    else if(source_is(0))
    {
        e.token = TOK_EOF;
    }
    else error("unknown char: '%c' (%i)", source_getc(), source_getc());

    _next = malloc(sizeof(expr_t) + strlen(token));
    memcpy(_next, &e, sizeof(expr_t));
    strcpy(_next->text, token);
    return _current;
}
