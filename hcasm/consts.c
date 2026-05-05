#include "asm.h"

const_t *_consts = NULL;

// [English] Check if a named constant exists in the linked list
// [Portuguese] Verifica se uma constante nomeada existe na lista encadeada
bool consts_exists(char *name)
{
    const_t *c = _consts;
    while (c)
    {
        if (is_keyword(c->name, name))
            return true;
        c = c->next;
    }
    return false;
}

// [English] Look up a named constant and return its value (0 if not found)
// [Portuguese] Procura uma constante nomeada e retorna seu valor (0 se não encontrada)
int consts_get(char *name)
{
    const_t *c = _consts;
    while (c)
    {
        if (is_keyword(c->name, name))
            return c->value;
        c = c->next;
    }
    return 0;
}

// [English] Set a named constant value, or create it if it does not exist
// [Portuguese] Define o valor de uma constante nomeada, ou a cria se não existir
void consts_set(char *name, int value)
{
    const_t *c = _consts;
    // [English] Search for existing constant
    // [Portuguese] Procura por constante existente
    while (c)
    {
        if (is_keyword(c->name, name))
        {
            c->value = value;
            return;
        }
        c = c->next;
    }
    // [English] Not found, allocate and prepend new node
    // [Portuguese] Não encontrada, aloca e insere novo nó no início
    c = malloc(sizeof(const_t) + strlen(name));
    c->next = _consts;
    c->value = value;
    strcpy(c->name, name);
    _consts = c;
}
