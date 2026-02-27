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

// --== Common ==--

typedef struct object_t
{
    struct object_t *next;
    char name[1];
} object_t;

// --== error.c ==-- 

// Emit error
void error(char *fmt, ...);