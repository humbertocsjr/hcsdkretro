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

#define ARGV_MAX 16

typedef struct source_t
{
    FILE *file;
    int c;
    int line;
    int column;
    char filename[1];
} source_t;

typedef enum token_t
{
    TOK_EOF,
    TOK_NEWLINE,
    TOK_SYMBOL,
    TOK_VALUE,
    TOK_COMMA,
    TOK_COLON,
    TOK_STRING,
    TOK_REGISTER,
    TOK_MNEMONIC,
    TOK_CURRENT_POS,
    TOK_ADD,
    TOK_SUB,
    TOK_MUL,
    TOK_DIV,
    TOK_MOD,
    TOK_INDEX_OPEN,
    TOK_INDEX_CLOSE,
    TOK_PARAMS_OPEN,
    TOK_PARAMS_CLOSE,
    TOK_SUB_LABEL
} token_t;

typedef struct reg_t
{
    char *name;
    int value;
    int value_aux;
    int group;
} reg_t;

typedef struct expr_t
{
    token_t token;
    int line;
    int column;
    int value;
    char *filename;
    reg_t *reg;
    bool force_byte;
    bool force_word;
    bool force_dword;
    bool force_qword;
    bool force_short;
    bool force_near;
    bool force_far;
    struct expr_t *left;
    struct expr_t *right;
    char text[1];
} expr_t;

typedef struct opcode_t
{
    char *mnemonic;
    int op1;
    int op2;
    int op3;
    int op4;
    int op5;
    int op6;
    void (*emit)(expr_t *mnemonic, struct opcode_t *opcode, int argc, expr_t *argv[]);

} opcode_t;

typedef struct const_t
{
    int value;
    struct const_t *next;
    char name[1];
} const_t;

// --== error.c ==--

// Emit Error
void error(char *fmt, ...);
// Emit Expression Error
void error_expr(expr_t *e, char *fmt, ...);


// --== main.c ==--

// current Sourc Fil
extern source_t *_source;

// --== obj.c ==--

// Open output file 
void out_open(char *name, char *dump_name);
// Close output file
void out_close();
// Write record to output
void out(rectype_t type, int16_t value, uint16_t aux, void *data, uint8_t data_size);

// --== source.c ==--

// Get next char
char source_nextc();
// Get current char
char source_getc();
// Get escaped char
char source_getescapec();
// Open source file
void source_open(char *filename);
// Close source file
void source_close();
// Verify if current char is equal a value
char source_is(char c);
// Verify if current char is between two values
char source_between(char min, char max);

// --== parser.c ==--

// Current label
extern char *_current_label;
// Parse source file
void parse(char *filename);

// --== scan.c ==--

// Scan next token
expr_t *scan();
// Get current token
expr_t *curr();
// Verify current token
bool curr_is(token_t token);
// Verify current token 
bool next_is(token_t token);
// Clone current token
expr_t *clone_expr(expr_t *src);
// Free expression tree
void free_expr(expr_t *e);
// Compare if current token is a keyword
bool curr_is_keyword(char *keyword);

// --== cpu/?????.c ==--

// CPU Type
extern rectype_t _cpu;
// CPU Registers
extern reg_t _regs[];
// CPU Prefix
extern opcode_t _prefix[];
// CPU Opcode
extern opcode_t _opcode[];

// --== expr.c ==--

// Filter out all registers
expr_t *filter_registers(expr_t *e);
// Validate if support size prefix
void validate(expr_t *e, bool support_byte, bool support_word, bool support_dword, bool support_qword);
// Validate if support distance prefix
void validate_distance(expr_t *e, bool support_short, bool support_near, bool support_far);
// Optimize Expression Tree
expr_t *optimize(expr_t *e);
// Generate Expression (OFFSET = BINARY OFFSET OF CURRENT MNEMONIC (EG: 0xAA $ = -1; 0xaa 0xbb $ = -2))
bool generate(expr_t *e, int offset, bool is_seg);

// --== consts.c ==--

// Verify if const exist
bool consts_exists(char *name);
// Get const value
int consts_get(char *name);
// Set const value
void consts_set(char *name, int value);