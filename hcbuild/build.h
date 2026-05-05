// [English] BSD 4-Clause License
// [Portuguese] Licenca BSD 4-Clausulas
// 
// [English] Copyright (c) 2025,2026, Humberto Costa dos Santos Junior
// [Portuguese] Copyright (c) 2025,2026, Humberto Costa dos Santos Junior
// [English] All rights reserved.
// [Portuguese] Todos os direitos reservados.

#pragma once

// [English] --== headers ==--
// [Portuguese] --== headers ==--

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "../include/obj.h"
#include "../include/version.h"

// [English] --== Common ==--
// [Portuguese] --== Common ==--

// [English] Linked list node for key-value pairs
// [Portuguese] No de par chave-valor para lista encadeada
typedef struct keyvalue_t
{
    // [English] Key name
    // [Portuguese] Nome da chave
    char *key;
    // [English] Value associated with the key
    // [Portuguese] Valor associado a chave
    char *value;
    // [English] Pointer to next node
    // [Portuguese] Ponteiro para o proximo no
    struct keyvalue_t *next;
} keyvalue_t;

// [English] Configuration section node
// [Portuguese] No de secao de configuracao
typedef struct section_t
{
    // [English] Section name
    // [Portuguese] Nome da secao
    char *name;
    // [English] Subsection name
    // [Portuguese] Nome da subsecao
    char *subsection;
    // [English] Linked list of key-value pairs
    // [Portuguese] Lista encadeada de pares chave-valor
    keyvalue_t *keys;
    // [English] Pointer to next section
    // [Portuguese] Ponteiro para a proxima secao
    struct section_t *next;
} section_t;

// [English] Object node (flexible struct with name array)
// [Portuguese] No de objeto (struct flexivel com array de nome)
typedef struct obj_t
{
    // [English] Pointer to next object
    // [Portuguese] Ponteiro para o proximo objeto
    struct obj_t *next;
    // [English] Object name (flexible array)
    // [Portuguese] Nome do objeto (array flexivel)
    char name[1];
} obj_t;

// [English] Include path node
// [Portuguese] No de caminho de inclusao
typedef struct include_path_t
{
    // [English] Include path string
    // [Portuguese] String do caminho de inclusao
    char path[1024];
    // [English] Pointer to next path
    // [Portuguese] Ponteiro para o proximo caminho
    struct include_path_t *next;
} include_path_t;

// [English] --== cfg.c ==--
// [Portuguese] --== cfg.c ==--

// [English] Process project configuration file
// [Portuguese] Processa o arquivo de configuracao do projeto
void cfg_process(char *filename);

// [English] --== section.c ==--
// [Portuguese] --== section.c ==--

// [English] Get section by name and subsection
// [Portuguese] Obtem a secao pelo nome e subsecao
section_t *get_section(char *name, char *subsection);

// [English] Get the value of a key within a section/subsection
// [Portuguese] Obtem o valor de uma chave dentro de uma secao/subsecao
char *get_value(char *name, char *subsection, char *key);

// [English] Get boolean value of a key (accepts: yes, true, 1, enable)
// [Portuguese] Obtem o valor booleano de uma chave (aceita: yes, true, 1, enable)
bool get_value_bool(char *name, char *subsection, char *key);
