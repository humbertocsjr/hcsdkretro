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

typedef enum nativetype_t
{
    NATIVETYPE_NONE,
    NATIVETYPE_8BITS,
    NATIVETYPE_16BITS,
    NATIVETYPE_24BITS,
    NATIVETYPE_32BITS,
    NATIVETYPE_STRUCTURE
} nativetype_t;

typedef struct datatype_t
{
    struct datatype_t *next;
    nativetype_t nativetype;
    bool is_signed;
    struct datatype_t *fields;
    uint16_t offset;
    int32_t size;
    char name[1];
} datatype_t;

typedef struct var_t
{
    struct var_t *next;
    datatype_t *datatype;
    bool is_pointer;
    uint8_t pointer_level;
    bool is_array;
    uint16_t array_size;
    bool is_global;
    int16_t local_offset;
    char name[1];
} var_t;


typedef struct func_t
{
    struct func_t *next;
    var_t *args;
    var_t *vars;
    char name[1];
} func_t;


// --== datatype.c ==--

// Calculate datatype size
void datatype_calcsize(datatype_t *datatype);
// Add Data Type
datatype_t *datatype_add(char *name, nativetype_t nativetype, bool is_signed);
// Add Structure
datatype_t *datatype_add_structure(char *name);
// Add Field to Structure
datatype_t *datatype_add_field(datatype_t *structure, char *name, nativetype_t nativetype, bool is_signed, int offset);
// Find Data Type
datatype_t *datatype_find(char *name);
// Find Field in Structure
datatype_t *datatype_find_field(datatype_t *structure, char *name);

// --== cpu/CPU.c ==--


// Initialize cpu
void cpu_init();
// Get compiler name extension
char *cpu_ext();
// Get format name
char *cpu_name();

// --== error.c ==--

// Emit error
void error(char *fmt, ...);