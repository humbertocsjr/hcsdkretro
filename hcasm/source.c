#include "asm.h"

#define SOURCES_MAX 32

static source_t *_sources[SOURCES_MAX];
static int _sources_current = -1;

// [English] Advance to next character, update line/column tracking, return it
// [Portuguese] Avança para o próximo caractere, atualiza rastreamento de linha/coluna, retorna
char source_nextc()
{
    _source->c = fgetc(_source->file);
    if (_source->c == EOF)
        _source->c = 0;
    _source->column++;
    if (_source->c == '\n')
        _source->line++;
    if (_source->c == '\n' || _source->c == '\r')
        _source->column = 0;
    if (_source->c == '\t')
        _source->column += 3;
    return _source->c;
}

// [English] Return current character without advancing
// [Portuguese] Retorna caractere atual sem avançar
char source_getc()
{
    return _source->c;
}

// [English] If current char is backslash, read the escape sequence and return decoded char
// [Portuguese] Se o char atual for barra invertida, lê a sequência de escape e retorna char decodificado
char source_getescapec()
{
    if (_source->c == '\\')
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

// [English] Check if current character equals a specific value
// [Portuguese] Verifica se o caractere atual é igual a um valor específico
char source_is(char c)
{
    return _source->c == c;
}

// [English] Check if current character is in a range [min, max]
// [Portuguese] Verifica se o caractere atual está em um intervalo [min, max]
char source_between(char min, char max)
{
    return _source->c >= min && _source->c <= max;
}

// [English] Open a source file and push it onto the include stack
// [Portuguese] Abre um arquivo fonte e o empilha na pilha de inclusão
void source_open(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
        error("can't open file: %s", filename);
    _sources_current++;
    if (_sources_current >= SOURCES_MAX)
        error("source files list overflow");
    _sources[_sources_current] = malloc(sizeof(source_t) + strlen(filename));
    _source = _sources[_sources_current];
    memset(_source, 0, sizeof(source_t));
    strcpy(_source->filename, filename);
    _source->file = file;
    _source->line = 1;
    _source->column = 0;
    source_nextc();
}

// [English] Close current source file, pop include stack, restore previous source
// [Portuguese] Fecha arquivo fonte atual, desempilha pilha de inclusão, restaura fonte anterior
void source_close()
{
    if (_source)
    {
        fclose(_source->file);
        free(_source);
    }
    _sources_current--;
    _source = NULL;
    if (_sources_current >= 0)
        _source = _sources[_sources_current];
}
