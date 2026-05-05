#include "bcomp.h"

static symbol_t *symtab = NULL;

// [English] Looks up a symbol by name in the symbol table using linear search.
// Returns the symbol pointer if found, or NULL if not found.
// [Portuguese] Busca um símbolo pelo nome na tabela de símbolos usando busca linear.
// Retorna o ponteiro do símbolo se encontrado, ou NULL se não encontrado.
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

// [English] Installs a new symbol into the symbol table with the given name,
// kind, size and segment. Allocates memory and inserts at the head of the list.
// [Portuguese] Instala um novo símbolo na tabela de símbolos com o nome,
// tipo, tamanho e segmento fornecidos. Aloca memória e insere no início da lista.
symbol_t *install(const char *name, symkind_t kind, int size, segkind_t seg)
{
    symbol_t *s = malloc(sizeof(symbol_t));
    s->name = malloc(strlen(name) + 1);
    strcpy(s->name, name);
    s->kind = kind;
    s->offset = 0;
    s->size = size;
    s->segment = seg;
    s->next = symtab;
    symtab = s;
    return s;
}
