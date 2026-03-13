#include "retrolang.h"

static source_t *_source = NULL;

source_t *source_get_list()
{
    return _source;
}

source_t *source_open(char *filename)
{
    source_t *obj = malloc(sizeof(source_t) + strlen(get_filename(filename)));
    if(!obj) error("Source memory overflow.");
    memset(obj, 0, sizeof(source_t));
    strcpy(obj->name, get_filename(filename));
    obj->file = fopen(filename, "r");
    if(!obj->file) error("can't open such file: %s", filename);
    obj->line = 1;
    source_next_char(obj);
    obj->next = _source;
    _source = obj;
    token_scan(obj);
    token_scan(obj);
    return obj;
}

void source_reset(source_t *source)
{
    fseek(source->file, 0, SEEK_SET);
}

void source_close(source_t *source)
{
    if(source->file) fclose(source->file);
    source->file = NULL;
}

char source_next_char(source_t *source)
{
    source->c = fgetc(source->file);
    if(source->c == EOF) source->c = 0;
    source->c_lower = tolower(source->c);
    source->column++;
    if(source->c == '\n' || source->c == '\r') source->column = 0;
    if(source->c == '\n') source->line++;
    return source->c;
}

char source_get_char(source_t *source)
{
    return source->c;
}

char source_get_escape_char(source_t *source)
{
    if(source->c == '\\')
    {
        source_next_char(source);
        switch(source->c)
        {
            case 'n': source->c = '\n'; break;
            case 't': source->c = '\t'; break;
            case 'r': source->c = '\r'; break;
            case 'a': source->c = '\a'; break;
            case 'b': source->c = '\b'; break;
            case 'e': source->c = 0x1b; break;
            default: break;
        }
    }
    return source->c;
}

char source_get_lower(source_t *source)
{
    return source->c_lower;
}

bool source_is_equal(source_t *source, char c)
{
    return source->c == c;
}

bool source_is_between(source_t *source, char min, char max)
{
    return source->c >= min && source->c <= max;
}

bool source_is_space(source_t *source)
{
    return 
        source_is_equal(source, ' ') ||
        source_is_equal(source, '\t') ||
        source_is_equal(source, '\r')
        ;
}

bool source_is_digit(source_t *source)
{
    return 
        source_is_between(source, '0', '9')
        ;
}

bool source_is_symbol(source_t *source)
{
    return 
        source_is_between(source, 'a', 'z') ||
        source_is_between(source, 'A', 'Z') ||
        source_is_between(source, '0', '9') ||
        source_is_equal(source, '_')
        ;
}

bool source_is_hexadecimal(source_t *source)
{
    return 
        source_is_between(source, 'a', 'f') ||
        source_is_between(source, 'A', 'f') ||
        source_is_between(source, '0', '9') ||
        source_is_equal(source, '_')
        ;
}

bool source_is_octal(source_t *source)
{
    return 
        source_is_between(source, '0', '7') ||
        source_is_equal(source, '_')
        ;
}

bool source_is_binary(source_t *source)
{
    return 
        source_is_between(source, '0', '1') ||
        source_is_equal(source, '_')
        ;
}

void source_concat_char(source_t *source)
{
    size_t len = strlen(source->next_token.token);
    if (len + 1 >= TOKEN_SIZE) 
    {
        return;
    }
    source->next_token.token[len] = source->c;
    source->next_token.token[len + 1] = '\0';
}

void source_concat_escaped_char(source_t *source)
{
    size_t len = strlen(source->next_token.token);
    if (len + 1 >= TOKEN_SIZE) 
    {
        return;
    }
    source->next_token.token[len] = source_get_escape_char(source);
    source->next_token.token[len + 1] = '\0';
}

void source_concat_lower_char(source_t *source)
{
    size_t len = strlen(source->next_token.token);
    if (len + 1 >= TOKEN_SIZE) 
    {
        return;
    }
    source->next_token.token[len] = source->c;
    source->next_token.token[len + 1] = '\0';
}