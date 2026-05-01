#include "bcomp.h"

static symbol_t *symtab = NULL;

symbol_t *lookup(const char *name)
{
    symbol_t *s = symtab;
    while (s)
    {
        if (!strcmp(s->name, name))
            return s;
        s = s->next;
    }
    return NULL;
}

symbol_t *install(const char *name, symkind_t kind, int size)
{
    symbol_t *s = malloc(sizeof(symbol_t));
    s->name = malloc(strlen(name) + 1);
    strcpy(s->name, name);
    s->kind = kind;
    s->offset = 0;
    s->size = size;
    s->next = symtab;
    symtab = s;
    return s;
}
