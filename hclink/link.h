// BSD 4-Clause License
//
// Copyright (c) 2025,2026, Humberto Costa dos Santos Junior
// All rights reserved.

#pragma once

// --== headers ==--

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

// --== common ==--

#define STACK_MAX 128
#define SECTIONS_MAX 512

typedef struct object_file_t
{
    struct object_file_t *next;
    FILE *file;
    fpos_t pos;
    fpos_t curr_rec;
    fpos_t next_rec;
    bool use_in_link;
    char name[1];
} object_file_t;

typedef struct section_t
{
    size_t start_default_pos;
    size_t start_pos;
    size_t position;
    size_t size;
    size_t align;
    rectype_t section;
    struct section_t *next;
    char name[1];
} section_t;

typedef enum step_t
{
    STEP_INITIALIZE,
    STEP_FILTER,
    STEP_PROCESS_FILES,
    STEP_VALIDATE,
    STEP_GENERATE
} step_t;

typedef struct const_t
{
    bool is_global;
    bool is_offset;
    int value;
    rectype_t section;
    struct const_t *next;
    bool changed;
    object_file_t *obj;
    char name[1];
} const_t;

// [English] Debug info structures for -dbg option
// [Portuguese] Estruturas de info de debug para opção -dbg
typedef struct file_info_t
{
    struct file_info_t *next;
    char name[1];
} file_info_t;

typedef struct line_info_t
{
    struct line_info_t *next;
    uint32_t address;
    uint16_t file_idx;
    uint16_t line;
    uint16_t column;
} line_info_t;

// --== link.c ==--

extern object_file_t *_objs;
extern object_file_t *_objs_last;
extern bool _multicpu;
extern rectype_t _cpu;
extern bool _verbose;
extern file_info_t *_debug_files;
extern line_info_t *_debug_lines;
extern const_t *_consts;

// --== format/?????.c ==--

// [English] Initialize format
// [Portuguese] Inicializa o formato
void format_init();
// [English] Get Format Name
// [Portuguese] Obtém o nome do formato
char *format_name();
// [English] Get Default File Extension
// [Portuguese] Obtém a extensão de arquivo padrão
char *format_ext();
// [English] Get Default Output File Name
// [Portuguese] Obtém o nome do arquivo de saída padrão
char *format_default_out_name();
// [English] Get Help Arguments
// [Portuguese] Obtém os argumentos de ajuda
void format_help_arguments();
// [English] Try Parse Argument
// [Portuguese] Tenta analisar um argumento
bool format_parse_arg(int argc, int *argi, char **argv);
// [English] Process step
// [Portuguese] Processa uma etapa
void format_process(step_t step);

// --== error.c ==--

// [English] Emit error
// [Portuguese] Emite um erro
void error(char *fmt, ...);

// --== section.c ==--

// [English] Get first section
// [Portuguese] Obtém a primeira seção
section_t *section_first();
// [English] Find section (return NULL if not found)
// [Portuguese] Localiza uma seção (retorna NULL se não encontrada)
section_t *section_find(char *name);
// [English] Create new section
// [Portuguese] Cria uma nova seção
section_t *section_new(char *name, char *prev_section, rectype_t section);
// [English] Set default align
// [Portuguese] Define o alinhamento padrão
void section_set_default_align(int align);
// [English] Reset section sizes
// [Portuguese] Redefine os tamanhos das seções
void section_reset_sizes();

// --== toint.c ==--

// [English] Convert string to int (support: binary, octal, hexadecimal, decimal)
// [Portuguese] Converte string para int (suporta: binário, octal, hexadecimal, decimal)
int toint(char *str);

// --== obj.c ==--

// [English] Add object file
// [Portuguese] Adiciona um arquivo-objeto
void obj_add(char *filename);
// [English] Read object record
// [Portuguese] Lê um registro de objeto
bool obj_read(object_file_t *file, record_t *rec);
// [English] Reset object record position to zero
// [Portuguese] Redefine a posição do registro de objeto para zero
void obj_reset(object_file_t *file);
// [English] Get object from constant
// [Portuguese] Obtém o objeto a partir de uma constante
object_file_t *consts_get_obj(object_file_t *obj, char *name);

// --== verbose.c ==--

// [English] Print if in verbose mode
// [Portuguese] Imprime se estiver no modo verbose
void verbose(char *fmt, ...);

// --== process.c ==--

// [English] Process
// [Portuguese] Processa
void process(step_t step);
// [English] Process objects
// [Portuguese] Processa objetos
size_t process_objs(step_t step, rectype_t section);
// [English] Emit error on process stages
// [Portuguese] Emite erro nas etapas de processamento
void process_error(char *fmt, ...);
// [English] Reorder objects so _start/_main is first in output
// [Portuguese] Reordena objetos para que _start/_main seja o primeiro na saída
void reorder_start_first(void);

// --== out.c ==--

// [English] Open output file
// [Portuguese] Abre o arquivo de saída
void out_open(char *name);
// [English] Close output file
// [Portuguese] Fecha o arquivo de saída
void out_close();
// [English] Write byte
// [Portuguese] Escreve um byte
void outb(int value);
// [English] Write word
// [Portuguese] Escreve uma word
void outw(int value);
// [English] Write data
// [Portuguese] Escreve dados
void out(void *data, int data_size);
// [English] Seek in output file
// [Portuguese] Posiciona no arquivo de saída
void out_seek(long offset, int whence);
// [English] Get current position in output file
// [Portuguese] Obtém a posição atual no arquivo de saída
long out_tell(void);

// [English] Format hooks (optional, can be NULL)
// [Portuguese] Hooks do formato (opcionais, podem ser NULL)
extern int (*format_adjust_value)(int value, int position, rectype_t section);
extern void (*format_record_reloc)(int position, rectype_t section);

// --== consts.c ==--

// [English] Verify if constant exists
// [Portuguese] Verifica se a constante existe
bool consts_exists(object_file_t *obj, char *name);
// [English] Set constant section
// [Portuguese] Define a seção da constante
void consts_set_section(object_file_t *obj, char *name, rectype_t section);
// [English] Get constant section
// [Portuguese] Obtém a seção da constante
rectype_t consts_get_section(object_file_t *obj, char *name);
// [English] Get constant value
// [Portuguese] Obtém o valor da constante
int consts_get(object_file_t *obj, char *name);
// [English] Set constant value
// [Portuguese] Define o valor da constante
void consts_set(object_file_t *obj, char *name, int value);
// [English] Reset changed flags
// [Portuguese] Redefine as flags de alteração
void consts_reset_changed();
// [English] Get changed status
// [Portuguese] Obtém o status de alteração
bool consts_is_changed();
// [English] Set constant global flag
// [Portuguese] Define a flag global da constante
void consts_set_global(object_file_t *obj, char *name);
// [English] Print symbols list
// [Portuguese] Imprime a lista de símbolos
void consts_print(char *sym_name);
// [English] Set constant as offset
// [Portuguese] Define a constante como offset
void consts_set_offset(object_file_t *obj, char *name);
// [English] Get constant is offset/label
// [Portuguese] Verifica se a constante é offset/rótulo
bool consts_is_offset(object_file_t *obj, char *name);
// [English] Emit consts keep changing error
// [Portuguese] Emite erro de constantes continuamente alteradas
void error_consts_has_changed();

// --== debug.c ==--

// [English] Add file to debug info
// [Portuguese] Adiciona arquivo a info de debug
int debug_add_file(char *filename);
// [English] Add line mapping to debug info
// [Portuguese] Adiciona mapeamento de linha a info de debug
void debug_add_line(uint32_t address, uint16_t file_idx, uint16_t line, uint16_t column);
// [English] Print debug info file
// [Portuguese] Imprime arquivo de info de debug
void debug_print(char *dbg_name);

// --== stack.c ==--

// [English] Push value to stack
// [Portuguese] Empurra valor para a pilha
void stack_push(int value);
// [English] Pop value from stack
// [Portuguese] Remove valor da pilha
int stack_pop();
// [English] Reset stack pointer
// [Portuguese] Redefine o ponteiro da pilha
void stack_reset();
