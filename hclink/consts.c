#include "link.h"

const_t *_consts = NULL;

// [English] Check if a constant exists, searching local first then global
// [Portuguese] Verifica se uma constante existe, primeiro local depois global
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

// [English] Check if a constant is an offset (memory address label)
// [Portuguese] Verifica se uma constante é um offset (rótulo de endereço de memória)
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

// [English] Get the object file that owns a given constant
// [Portuguese] Obtém o arquivo objeto que possui uma determinada constante
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

// [English] Get the numeric value of a constant by name
// [Portuguese] Obtém o valor numérico de uma constante pelo nome
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

// [English] Set or create a constant. Updates existing value or allocates a new entry.
// [Portuguese] Define ou cria uma constante. Atualiza valor existente ou aloca uma nova entrada.
void consts_set(object_file_t *obj, char *name, int value)
{
    // [English] Search for existing constant in the linked list
    // [Portuguese] Procura constante existente na lista encadeada
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
    // [English] Create a new constant if not found
    // [Portuguese] Cria uma nova constante se não encontrada
    c = malloc(sizeof(const_t) + strlen(name));
    c->next = _consts;
    c->is_global = false;
    c->value = value;
    c->obj = obj;
    c->changed = true;
    strcpy(c->name, name);
    _consts = c;
}

// [English] Mark a constant as an offset (label pointing to a position in memory)
// [Portuguese] Marca uma constante como offset (rótulo apontando para uma posição na memória)
void consts_set_offset(object_file_t *obj, char *name)
{
    // [English] Find existing constant and mark as offset
    // [Portuguese] Encontra constante existente e marca como offset
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
    // [English] Create new offset constant entry
    // [Portuguese] Cria nova entrada de constante de offset
    c = malloc(sizeof(const_t) + strlen(name));
    c->next = _consts;
    c->is_offset = true;
    c->value = 0;
    c->obj = obj;
    c->changed = true;
    strcpy(c->name, name);
    _consts = c;
}

// [English] Mark a constant as global (accessible across all object files)
// [Portuguese] Marca uma constante como global (acessível entre todos os arquivos objeto)
void consts_set_global(object_file_t *obj, char *name)
{
    // [English] Find existing constant and mark as global
    // [Portuguese] Encontra constante existente e marca como global
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
    // [English] Create new global constant entry
    // [Portuguese] Cria nova entrada de constante global
    c = malloc(sizeof(const_t) + strlen(name));
    c->next = _consts;
    c->is_global = true;
    c->value = 0;
    c->obj = obj;
    c->changed = true;
    strcpy(c->name, name);
    _consts = c;
}

// [English] Set the section type associated with a constant
// [Portuguese] Define o tipo de seção associado a uma constante
void consts_set_section(object_file_t *obj, char *name, rectype_t section)
{
    // [English] Find existing constant and update its section
    // [Portuguese] Encontra constante existente e atualiza sua seção
    const_t *c = _consts;
    while (c)
    {
        if (c->obj == obj && !strcmp(c->name, name))
        {
            c->section = section;
            return;
        }
        c = c->next;
    }
    // [English] Create new constant with section metadata
    // [Portuguese] Cria nova constante com metadados de seção
    c = malloc(sizeof(const_t) + strlen(name));
    c->next = _consts;
    c->section = section;
    c->is_global = false;
    c->value = 0;
    c->obj = obj;
    c->changed = true;
    strcpy(c->name, name);
    _consts = c;
}

// [English] Get the section type associated with a constant
// [Portuguese] Obtém o tipo de seção associado a uma constante
rectype_t consts_get_section(object_file_t *obj, char *name)
{
    const_t *c = _consts;
    while (c)
    {
        if (c->obj == obj && !strcmp(c->name, name))
            return c->section;
        c = c->next;
    }
    c = _consts;
    while (c)
    {
        if (c->is_global && !strcmp(c->name, name))
            return c->section;
        c = c->next;
    }
    return 0;
}

// [English] Reset the changed flag on all constants
// [Portuguese] Reseta a flag de alteração em todas as constantes
void consts_reset_changed()
{
    const_t *c = _consts;
    while (c)
    {
        c->changed = false;
        c = c->next;
    }
}

// [English] Report error when constants keep changing after multiple processing passes
// [Portuguese] Reporta erro quando constantes continuam mudando após múltiplas passadas
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

// [English] Check if any constant has been flagged as changed
// [Portuguese] Verifica se alguma constante foi marcada como alterada
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

// [English] Print all constants to a symbol file for debugging
// [Portuguese] Imprime todas as constantes em um arquivo de símbolos para depuração
void consts_print(char *sym_name)
{
    if (!sym_name)
        return;
    // [English] Open the symbol output file
    // [Portuguese] Abre o arquivo de saída de símbolos
    FILE *sym = fopen(sym_name, "w");
    if (!sym)
    {
        error("can't open symbol file: %s", sym_name);
    }
    // [English] Write global symbols first
    // [Portuguese] Escreve símbolos globais primeiro
    const_t *c = _consts;
    while (c)
    {
        if (c->is_global && c->obj->use_in_link)
        {
            fprintf(sym, "$%04X %s [GLOBAL %s %s]\n", c->value, c->name, c->is_offset ? "LABEL" : "CONST", c->obj->name);
        }
        c = c->next;
    }
    // [English] Write local symbols second
    // [Portuguese] Escreve símbolos locais em segundo
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
