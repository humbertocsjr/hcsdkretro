#include "link.h"

static section_t *_sections[SECTIONS_MAX];
static int _sections_next = 0;
static section_t *_section_first = 0;
static int _sections_align = 16;

// [English] Return the first section in the linked list
// [Portuguese] Retorna a primeira seção na lista encadeada
section_t *section_first()
{
    return _section_first;
}

// [English] Set the default alignment for all existing sections
// [Portuguese] Define o alinhamento padrão para todas as seções existentes
void section_set_default_align(int align)
{
    if (align < 0)
        error("invalid default align: %i", align);
    _sections_align = align;
    for (int i = 0; i < _sections_next; i++)
    {
        if (_sections[i])
            _sections[i]->align = align;
        ;
    }
}

// [English] Reset the size and position of all sections to their start positions
// [Portuguese] Reinicia o tamanho e a posição de todas as seções para suas posições iniciais
void section_reset_sizes()
{
    for (int i = 0; i < _sections_next; i++)
    {
        _sections[i]->size = 0;
        _sections[i]->position = _sections[i]->start_pos;
    }
}

// [English] Find a section by name in the sections array
// [Portuguese] Encontra uma seção pelo nome no array de seções
section_t *section_find(char *name)
{
    if (!name)
        return NULL;
    for (int i = 0; i < _sections_next; i++)
    {
        if (!strcmp(_sections[i]->name, name))
            return _sections[i];
    }
    return NULL;
}

// [English] Create a new section with a given name, placing it after a previous section
// [Portuguese] Cria uma nova seção com um nome dado, colocando-a após uma seção anterior
section_t *section_new(char *name, char *prev_section, rectype_t section)
{
    if (_sections_next >= SECTIONS_MAX)
        error("section list overflow.");
    section_t *s = malloc(sizeof(section_t) + strlen(name));
    section_t *prev;
    // [English] Find the previous section to link after it
    // [Portuguese] Encontra a seção anterior para vincular após ela
    if (prev_section)
    {
        prev = section_find(prev_section);
        if (!prev)
            error("section not found: %s", prev_section);
    }
    else
    {
        prev = _section_first;
    }
    // [English] Initialize the new section with default values
    // [Portuguese] Inicializa a nova seção com valores padrão
    memset(s, 0, sizeof(section_t));
    s->align = _sections_align;
    s->next = prev ? prev->next : NULL;
    if (prev)
        prev->next = s;
    else
        _section_first = s;
    s->size = 0;
    s->section = section;
    s->start_pos = 0;
    s->start_default_pos = 0;
    strcpy(s->name, name);
    _sections[_sections_next++] = s;
    return s;
}
