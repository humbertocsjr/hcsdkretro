#include "asm.h"

const_t *_consts = NULL;

bool consts_exists(char *name)
{
    const_t *c = _consts;
    while(c)
    {
        if(is_keyword(c->name, name)) return true;
        c = c->next;
    }
    return false;
}

int consts_get(char *name)
{
    const_t *c = _consts;
    while(c)
    {
        if(is_keyword(c->name, name)) return c->value;
        c = c->next;
    }
    return 0;
}

void consts_set(char *name, int value)
{
    const_t *c = _consts;
    while(c)
    {
        if(is_keyword(c->name, name))
        {
            c->value = value;
            return;
        }
        c = c->next;
    }
    c = malloc(sizeof(const_t) + strlen(name));
    c->next = _consts;
    c->value = value;
    strcpy(c->name, name);
    _consts = c;
}