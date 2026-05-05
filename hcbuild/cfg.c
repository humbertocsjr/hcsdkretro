#include "build.h"

// [English] Internal linked list of parsed sections
// [Portuguese] Lista encadeada interna de secoes analisadas
section_t *_section = NULL;

// [English] Trim leading whitespace from a string
// [Portuguese] Remove espacos em branco do inicio de uma string
char *ltrim(char *s) 
{
    while(isspace((unsigned char)*s)) 
    {
        s++;
    }
    return s;
}

// [English] Trim trailing whitespace from a string
// [Portuguese] Remove espacos em branco do final de uma string
char *rtrim(char *s) 
{
    if(!s) return s;
    char *back = s + strlen(s) - 1;
    while(back >= s && isspace((unsigned char)*back)) 
    {
        back--;
    }
    if(*s == 0) return s;
    *(back+1) = '\0';
    return s;
}

// [English] Find an existing section by name and subsection
// [Portuguese] Localiza uma secao existente pelo nome e subsecao
section_t *get_section(char *name, char *subsection)
{
    section_t *section = _section;
    while(section)
    {
        if(!strcmp(name, section->name) && !strcmp(subsection, section->subsection))
        {
            return section;
        }
        section = section->next;
    }
    return NULL;
}

// [English] Add a new section (or return existing one) with trimmed name/subsection
// [Portuguese] Adiciona uma nova secao (ou retorna existente) com nome/subsecao tratados
section_t *add_section(char *name, char *subsection)
{
    // [English] Check if section already exists
    // [Portuguese] Verifica se a secao ja existe
    char *tmp;
    section_t *section = _section;
    while(section)
    {
        if(!strcmp(name, section->name) && !strcmp(subsection, section->subsection))
        {
            return section;
        }
        section = section->next;
    }

    // [English] Allocate and initialize new section
    // [Portuguese] Aloca e inicializa nova secao
    section = malloc(sizeof(section_t));
    section->next = _section;
    _section = section;

    // [English] Trim whitespace from name and subsection
    // [Portuguese] Remove espacos em branco do nome e subsecao
    tmp = rtrim(ltrim(name));
    section->name = malloc(strlen(tmp) + 1);
    strcpy(section->name, tmp);
    tmp = rtrim(ltrim(subsection));
    section->subsection = malloc(strlen(tmp) + 1);
    strcpy(section->subsection, tmp);
    section->keys = NULL;
    return section;
}

// [English] Get the value of a key searching across all sections
// [Portuguese] Obtem o valor de uma chave pesquisando em todas as secoes
char *get_value(char *name, char *subsection, char *key)
{
    // [English] Search through all sections for matching name/subsection
    // [Portuguese] Percorre todas as secoes procurando nome/subsecao correspondentes
    section_t *section = _section;
    keyvalue_t *kv;
    while(section)
    {
        if(!strcmp(name, section->name) && !strcmp(subsection, section->subsection))
        {
            // [English] Search through key-value pairs in the matching section
            // [Portuguese] Percorre os pares chave-valor na secao correspondente
            kv = section->keys;
            while(kv)
            {
                if(!strcmp(key, kv->key))
                {
                    return kv->value;
                }
                kv = kv->next;
            }
        }
        section = section->next;
    }
    return "";
}

// [English] Get boolean interpretation of a key's value
// [Portuguese] Obtem interpretacao booleana do valor de uma chave
bool get_value_bool(char *name, char *subsection, char *key)
{
    // [English] Convert value to lowercase for comparison
    // [Portuguese] Converte o valor para minusculo para comparacao
    char *value = get_value(name, subsection, key);
    char *ptr = value;
    while(*ptr)
    {
        *ptr = tolower(*ptr);
        ptr++;
    }
    if(!strcmp(value, "yes") || !strcmp(value, "true") || !strcmp(value, "1") || !strcmp(value, "enable")) return true;
    return false;
}

// [English] Set or update a key-value pair in the given section
// [Portuguese] Define ou atualiza um par chave-valor na secao fornecida
void set_key(section_t *section, char *key, char *value)
{
    char *tmp;

    // [English] Validate section pointer
    // [Portuguese] Valida o ponteiro da secao
    if(!section)
    {
        fprintf(stderr, "error: can't add key without section: %s\n", key);
        exit(1);
    }

    // [English] Search for existing key to update its value
    // [Portuguese] Procura chave existente para atualizar seu valor
    keyvalue_t *kv = section->keys;
    while(kv)
    {
        if(!strcmp(key, kv->key))
        {
            if(kv->value) free(kv->value);
            kv->value = malloc(strlen(value) + 1);
            strcpy(kv->value, value);
            return;
        }
        kv = kv->next;
    }

    // [English] Create new key-value node
    // [Portuguese] Cria novo no chave-valor
    kv = malloc(sizeof(keyvalue_t));
    tmp = rtrim(ltrim(key));
    kv->key = malloc(strlen(tmp) + 1);
    strcpy(kv->key, tmp);
    tmp = rtrim(ltrim(value));
    kv->value = malloc(strlen(tmp) + 1);
    strcpy(kv->value, tmp);
    kv->next = section->keys;
    section->keys = kv;
}

// [English] Parse and process a configuration file
// [Portuguese] Analisa e processa um arquivo de configuracao
void cfg_process(char *filename)
{
    char key[1024];
    char value[1024];
    int key_i = 0;
    int value_i = 0;
    int step = 0;
    int c = 0;
    section_t *section = NULL;

    // [English] -- Open configuration file --
    // [Portuguese] -- Abre arquivo de configuracao --
    FILE *cfg = fopen(filename, "r");
    if(!cfg)
    {
        fprintf(stderr, "error: can't open file: %s\n", filename);
        exit(1);
    }

    // [English] -- Main parsing loop --
    // [Portuguese] -- Loop principal de analise --
    while(1)
    {
        c = fgetc(cfg);
        if(c == EOF) break;

        // [English] -- Overflow checks --
        // [Portuguese] -- Verificacoes de estouro --
        if(key_i >= 1023)
        {
            fprintf(stderr, "error: key size overflow: %s\n", key);
            exit(1);
        }
        if(value_i >= 1023)
        {
            fprintf(stderr, "error: value size overflow: %s = %s\n", key, value);
            exit(1);
        }

        // [English] -- Skip comments (; or #) --
        // [Portuguese] -- Ignora comentarios (; ou #) --
        if(c == ';' || c == '#')
        {
            while(c != '\n') c = fgetc(cfg);
        }

        // [English] -- State machine --
        // [Portuguese] -- Maquina de estados --
        switch (step)
        {
            // [English] Initial state
            // [Portuguese] Estado inicial
            case 0:
                {
                    strcpy(key, "");
                    strcpy(value, "");
                    key_i = 0;
                    value_i = 0;
                    if(c == '[')
                    {
                        step = 1;
                    }
                    else if(c != ' ' && c != '\t' && c != '\r' && c != '\n')
                    {
                        step = 3;
                        key[key_i++] = c;
                        key[key_i] = 0;
                    }
                }
                break;
            // [English] Section name
            // [Portuguese] Nome da secao
            case 1:
                {
                    if(c == ':')
                    {
                        step = 2;
                    }
                    else if(c == ']')
                    {
                        step = 0;
                        section = add_section(key, "");
                    }
                    else
                    {
                        key[key_i++] = c;
                        key[key_i] = 0;
                    }
                }
                break;
            // [English] Subsection
            // [Portuguese] Subsecao
            case 2:
                {
                    if(c == ']')
                    {
                        step = 0;
                        section = add_section(key, value);
                    }
                    else
                    {
                        value[value_i++] = c;
                        value[value_i] = 0;
                    }
                }
                break;
            // [English] Key
            // [Portuguese] Chave
            case 3:
                {
                    if(c == '=')
                    {
                        step = 4;
                    }
                    else if(c == '\n' || c == '\r' || c == EOF)
                    {
                        step = 0;
                        set_key(section, key, "");
                    }
                    else
                    {
                        key[key_i++] = c;
                        key[key_i] = 0;
                    }
                }
                break;
            // [English] Value
            // [Portuguese] Valor
            case 4:
                {
                    if(c == '"')
                    {
                        step = 5;
                    }
                    else if(c == '\n' || c == '\r' || c == EOF)
                    {
                        step = 0;
                        set_key(section, key, value);
                    }
                    else
                    {
                        value[value_i++] = c;
                        value[value_i] = 0;
                    }
                    
                }
                break;
            // [English] Quoted value
            // [Portuguese] Valor entre aspas
            case 5:
                {
                    if(c == '"')
                    {
                        step = 0;
                        set_key(section, key, value);
                    }
                    else if(c == '\n' || c == '\r' || c == EOF)
                    {
                        fprintf(stderr, "error: unterminated quoted value\n");
                        exit(1);
                    }
                    else
                    {
                        value[value_i++] = c;
                        value[value_i] = 0;
                    }
                }
                break;
        }
    }

    // [English] -- Finalize remaining state --
    // [Portuguese] -- Finaliza estado restante --
    if(step == 1 || step == 2)
    {
        add_section(key, value);
    }
    else if(step == 3 || step == 4 || step == 5)
    {
        set_key(section, key, value);
    }
    fclose(cfg);
}
