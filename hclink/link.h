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
    struct const_t *next;
    bool changed;
    object_file_t *obj;
    char name[1];
} const_t;

// --== link.c ==--

extern object_file_t *_objs;
extern object_file_t *_objs_last;
extern bool _multicpu;
extern rectype_t _cpu;
extern bool _verbose;

// --== format/?????.c ==--

// Initialize format
void format_init();
// Get Format Name
char *format_name();
// Get Default File Extension
char *format_ext();
// Get Default Output File Name
char *format_default_out_name();
// Get Help Arguments
void format_help_arguments();
// Try Parse Argument
bool format_parse_arg(int argc, int *argi, char **argv);
// Process step
void format_process(step_t step);

// --== error.c ==--

// Emit error
void error(char *fmt, ...);

// --== section.c ==--

// Get first section
section_t *section_first();
// Find section (return NULL if not found)
section_t *section_find(char *name);
// Create new section
section_t *section_new(char *name, char *prev_section, rectype_t section);
// Set default align
void section_set_default_align(int align);
// Reset section sizes
void section_reset_sizes();

// --== toint.c ==--

// Convert string to int (support: binary, octal, hexadecimal, decimal)
int toint(char *str);

// --== obj.c ==--

// Add object file
void obj_add(char *filename);
// Read object record
bool obj_read(object_file_t *file, record_t *rec);
// Reset object record position to zero
void obj_reset(object_file_t *file);
// Get object from constant
object_file_t *consts_get_obj(object_file_t *obj, char *name);

// --== verbose.c ==--

// Print if in verbose mode
void verbose(char *fmt, ...);

// --== process.c ==--
// Process
void process(step_t step);
// Process objects
size_t process_objs(step_t step, rectype_t section);
// Emit error on process stages
void process_error(char *fmt, ...);
// Reorder objects so _start/_main is first in output
void reorder_start_first(void);

// --== out.c ==--

// Open output file
void out_open(char *name);
// Close output file
void out_close();
// Write byte
void outb(int value);
// Write word
void outw(int value);
// Write data
void out(void *data, int data_size);

// --== consts.c ==--

// Verify if constant exists
bool consts_exists(object_file_t *obj, char *name);
// Get constant value
int consts_get(object_file_t *obj, char *name);
// Set constant value
void consts_set(object_file_t *obj, char *name, int value);
// Reset changed flags
void consts_reset_changed();
// Get changed status
bool consts_is_changed();
// Set constant global flag
void consts_set_global(object_file_t *obj, char *name);
// Print symbols list
void consts_print(char *sym_name);
// Set constant as offset
void consts_set_offset(object_file_t *obj, char *name);
// Get constant is offset/label
bool consts_is_offset(object_file_t *obj, char *name);
// Emit consts keep changing error
void error_consts_has_changed();

// --== stack.c ==--

// Push value to stack
void stack_push(int value);
// Pop value from stack
int stack_pop();
// Reset stack pointer
void stack_reset();