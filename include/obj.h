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
#include <string.h>

// --== common ==--

#pragma pack(1)

typedef enum rectype_t
{
    // File info
    REC_FILENAME, // DATA = FILENAME
    REC_POSITION, // VALUE = LINE, AUX = COLUMN
    REC_END_OF_FILE, // DATA = FILENAME
    // Section info
    REC_SECTION_TEXT = 0x10,
    REC_SECTION_DATA,
    REC_SECTION_BSS,
    REC_SECTION_RELOC,
    // Data
    REC_DATA = 0x20,
    REC_DATA_RESERVE, // VALUE = SIZE
    // Constant declaration
    REC_CONST_LABEL = 0x30, // DATA = NAME
    REC_CONST_CUSTOM, // DATA = NAME, VALUE = INITIAL VALUE
    REC_CONST_AS_GLOBAL_LABEL, // DATA = NAME
    // Expressions
    REC_EXPR_RESET = 0x40,
    REC_EXPR_PUSH_VALUE, // VALUE = VALUE
    REC_EXPR_PUSH_VALUE_UNSIGNED, // VALUE = UNSIGNED VALUE
    REC_EXPR_PUSH_OFFSET, // CURRENT OFFSET
    REC_EXPR_PUSH_CONST, // DATA = CONSTANT NAME
    REC_EXPR_PUSH_SEGMENT, // DATA = LABEL NAME
    REC_EXPR_ADD,
    REC_EXPR_SUB,
    REC_EXPR_MUL,
    REC_EXPR_DIV,
    REC_EXPR_MOD,
    REC_EXPR_SHL,
    REC_EXPR_SHR,
    REC_EXPR_NOT,
    REC_EXPR_AND,
    REC_EXPR_OR,
    REC_EXPR_XOR,
    REC_EXPR_POP_TO_CONST,
    REC_EXPR_POP_INT8_EMIT,
    REC_EXPR_POP_INT16_EMIT,
    REC_EXPR_POP_INT16_RELOCATABLE_EMIT,
    REC_EXPR_POP_REPEAT_TIMES,
    REC_EXPR_REPEAT_TIMES_END,
    // CPU IDENTIFICATIO
    REC_CPU = 0xf0,
    REC_CPU_8080 = 0xf0, // intel 8080
    REC_CPU_8085, // intel 8085
    REC_CPU_Z80, // zilog z80
    REC_CPU_8086, // intel 8086
    MASK_REC_TYPE = 0xf,
} rectype_t;

typedef struct record_header_t
{
    uint8_t type;
    uint8_t data_size;
    int16_t value;
    uint16_t aux;
} record_header_t;

typedef struct record_t
{
    record_header_t header;
    uint8_t data[256];
} record_t;

#pragma pack()
