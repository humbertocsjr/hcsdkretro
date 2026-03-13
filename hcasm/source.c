#include "asm.h"

#define SOURCES_MAX 32

static source_t *_sources[SOURCES_MAX];
static int _sources_current = -1;

char source_nextc()
{
    _source->c = fgetc(_source->file);
    if(_source->c == EOF) _source->c = 0;
    _source->column++;
    if(_source->c == '\n') _source->line ++;
    if(_source->c == '\n' || _source->c == '\r') _source->column = 0;
    if(_source->c == '\t') _source->column += 3;
    return _source->c;
}

char source_getc()
{
    return _source->c;
}

char source_getescapec()
{
    if(_source->c == '\\')
    {
        source_nextc();
        switch (_source->c)
        {
            case 'n': 
                _source->c = '\n';
                break;
            case 'r': 
                _source->c = '\r';
                break;
            case 't': 
                _source->c = '\t';
                break;
            case 'a': 
                _source->c = '\a';
                break;
            case 'b': 
                _source->c = '\b';
                break;
        }
    }
    return _source->c;
}

char source_is(char c)
{
    return _source->c == c;
}

char source_between(char min, char max)
{
    return _source->c >= min && _source->c <= max;
}

void source_open(char *filename)
{
    FILE *file = fopen(filename, "r");
    if(!file) error("can't open file: %s", filename);
    _sources_current++;
    if(_sources_current >= SOURCES_MAX) error("source files list overflow");
    _sources[_sources_current] = malloc(sizeof(source_t) + strlen(filename));
    _source = _sources[_sources_current];
    memset(_source, 0, sizeof(source_t));
    strcpy(_source->filename, filename);
    _source->file = file;
    _source->line = 1;
    _source->column = 0;
    source_nextc();
}

void source_close()
{
    fclose(_source->file);
    free(_source);
    _sources_current--;
    _source = NULL;
    if(_sources_current >= 0) _source = _sources[_sources_current];
}