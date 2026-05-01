#include "link.h"

const_t *_consts = NULL;

bool consts_exists(object_file_t *obj, char *name)
{
    const_t *c = _consts;
    while (c)
    {
        if (c->obj == obj && !strcmp(c->name, name))
            return true;
        c = c->next;
    }
    c = _consts;
    while (c)
    {
        if (c->is_global && !strcmp(c->name, name))
            return true;
        c = c->next;
    }
    return false;
}

bool consts_is_offset(object_file_t *obj, char *name)
{
    const_t *c = _consts;
    while (c)
    {
        if (c->obj == obj && !strcmp(c->name, name))
            return c->is_offset;
        c = c->next;
    }
    c = _consts;
    while (c)
    {
        if (c->is_global && !strcmp(c->name, name))
            return c->is_offset;
        c = c->next;
    }
    return false;
}

object_file_t *consts_get_obj(object_file_t *obj, char *name)
{
    const_t *c = _consts;
    while (c)
    {
        if (c->obj == obj && !strcmp(c->name, name))
            return c->obj;
        c = c->next;
    }
    c = _consts;
    while (c)
    {
        if (c->is_global && !strcmp(c->name, name))
            return c->obj;
        c = c->next;
    }
    return 0;
}

int consts_get(object_file_t *obj, char *name)
{
    const_t *c = _consts;
    while (c)
    {
        if (c->obj == obj && !strcmp(c->name, name))
            return c->value;
        c = c->next;
    }
    c = _consts;
    while (c)
    {
        if (c->is_global && !strcmp(c->name, name))
            return c->value;
        c = c->next;
    }
    return 0;
}

void consts_set(object_file_t *obj, char *name, int value)
{
    const_t *c = _consts;
    while (c)
    {
        if (c->obj == obj && !strcmp(c->name, name))
        {
            if (c->value != value)
            {
                c->changed = true;
            }
            c->value = value;
            return;
        }
        c = c->next;
    }
    c = malloc(sizeof(const_t) + strlen(name));
    c->next = _consts;
    c->is_global = false;
    c->value = value;
    c->obj = obj;
    c->changed = true;
    strcpy(c->name, name);
    _consts = c;
}

void consts_set_offset(object_file_t *obj, char *name)
{
    const_t *c = _consts;
    while (c)
    {
        if (c->obj == obj && !strcmp(c->name, name))
        {
            c->is_offset = true;
            return;
        }
        c = c->next;
    }
    c = malloc(sizeof(const_t) + strlen(name));
    c->next = _consts;
    c->is_offset = true;
    c->value = 0;
    c->obj = obj;
    c->changed = true;
    strcpy(c->name, name);
    _consts = c;
}

void consts_set_global(object_file_t *obj, char *name)
{
    const_t *c = _consts;
    while (c)
    {
        if (c->obj == obj && !strcmp(c->name, name))
        {
            c->is_global = true;
            return;
        }
        c = c->next;
    }
    c = malloc(sizeof(const_t) + strlen(name));
    c->next = _consts;
    c->is_global = true;
    c->value = 0;
    c->obj = obj;
    c->changed = true;
    strcpy(c->name, name);
    _consts = c;
}

void consts_reset_changed()
{
    const_t *c = _consts;
    while (c)
    {
        c->changed = false;
        c = c->next;
    }
}

void error_consts_has_changed()
{
    const_t *c = _consts;
    while (c)
    {
        if (c->changed)
            break;
        c = c->next;
    }
    error("fail to process files, labels keep changing: %s", c->name);
}

bool consts_is_changed()
{
    const_t *c = _consts;
    while (c)
    {
        if (c->changed)
            return true;
        c = c->next;
    }
    return false;
}

void consts_print(char *sym_name)
{
    if (!sym_name)
        return;
    FILE *sym = fopen(sym_name, "w");
    if (!sym)
    {
        error("can't open symbol file: %s", sym_name);
    }
    const_t *c = _consts;
    while (c)
    {
        if (c->is_global && c->obj->use_in_link)
        {
            fprintf(sym, "$%04X %s [GLOBAL %s %s]\n", c->value, c->name, c->is_offset ? "LABEL" : "CONST", c->obj->name);
        }
        c = c->next;
    }
    c = _consts;
    while (c)
    {
        if (!c->is_global && c->obj->use_in_link)
        {
            fprintf(sym, "$%04X %s [LOCAL %s %s]\n", c->value, c->name, c->is_offset ? "LABEL" : "CONST", c->obj->name);
        }
        c = c->next;
    }
    fclose(sym);
}