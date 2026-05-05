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

typedef struct object_t
{
    struct object_t *next;
    char name[1];
} object_t;

// [English] --== error.c ==--
// [Portuguese] --== error.c ==--

// [English] Emit error
// [Portuguese] Emite um erro
void error(char *fmt, ...);
