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

// --== Common ==--

typedef struct keyvalue_t
{
    char *key;
    char *value;
    struct keyvalue_t *next;
} keyvalue_t;

typedef struct section_t
{
    char *name;
    char *subsection;
    keyvalue_t *keys;
    struct section_t *next;
} section_t;

typedef struct obj_t
{
    struct obj_t *next;
    char name[1];
} obj_t;

typedef struct include_path_t
{
    char path[1024];
    struct include_path_t *next;
} include_path_t;

// --== cfg.c ==--

// Process project file
void cfg_process(char *filename);

// --== section.c ==--

// Get section
section_t *get_section(char *name, char *subsection);
// Get key value
char *get_value(char *name, char *subsection, char *key);
// Get key bool value
bool get_value_bool(char *name, char *subsection, char *key);