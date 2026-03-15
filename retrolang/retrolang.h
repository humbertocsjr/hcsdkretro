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
    struct var_t *fields;
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
    struct command_t *contents;
    bool is_external;
    bool is_global_context;
    char name[1];
} func_t;

typedef enum tok_t
{
    TOK_EOF,
    TOK_NEWLINE,
    TOK_SYMBOL,
    TOK_INTEGER,
    TOK_STRING,
    TOK_ADD,
    TOK_ADD_ASSIGN,
    TOK_SUB,
    TOK_SUB_ASSIGN,
    TOK_MUL,
    TOK_MUL_ASSIGN,
    TOK_DIV,
    TOK_DIV_ASSIGN,
    TOK_INLINE_ELSE,
    TOK_ASSIGN,
    TOK_SINGLE_EQUAL,
    TOK_EQUAL,
    TOK_LESS_THAN,
    TOK_NOT_EQUAL,
    TOK_LESS_OR_EQUAL,
    TOK_SHIFT_LEFT,
    TOK_SHIFT_LEFT_ASSIGN,
    TOK_GREATER_THAN,
    TOK_GREATER_OR_EQUAL,
    TOK_SHIFT_RIGHT,
    TOK_SHIFT_RIGHT_ASSIGN,
    TOK_INLINE_IF,
    TOK_POINTER,
    TOK_INDEX_OPEN,
    TOK_INDEX_CLOSE,
    TOK_PARAMS_OPEN,
    TOK_PARAMS_CLOSE,
    TOK_COMMA,
    TOK_MOD,
    TOK_MOD_ASSIGN,
    TOK_OR,
    TOK_OR_ELSE,
    TOK_AND,
    TOK_AND_ALSO,
    TOK_XOR,
    TOK_XOR_ASSIGN,
    KEY_DEF,
    KEY_IF,
    KEY_UNTIL,
    KEY_ELSE,
    KEY_WHILE,
    KEY_FOR,
    KEY_FOREACH,
    KEY_END,
    KEY_VAR,
    KEY_AS,
    KEY_TYPEDEF,
    KEY_ASM,
    ACT_VARIABLE,
    ACT_FUNCTION,
    ACT_CALL,
    ACT_INDEXED,
    ACT_INDEXED_CALL
} tok_t;

#define TOKEN_SIZE 256

typedef struct token_t
{
    tok_t tok;
    struct source_t *source;
    uint16_t line;
    uint16_t column;
    int32_t value;
    char token[TOKEN_SIZE];
} token_t;

typedef struct source_t
{
    uint16_t line;
    uint16_t column;
    int c;
    int c_lower;
    FILE *file;
    token_t curr_token;
    token_t next_token;
    struct source_t *next;
    char name[1];
} source_t;

typedef struct expr_t
{
    tok_t tok;
    struct source_t *source;
    uint16_t line;
    uint16_t column;
    int32_t value;
    struct command_t *command;
    struct expr_t *left;
    struct expr_t *right;
    char token[1];
} expr_t;

typedef enum cmd_t
{
    CMD_NONE,
    CMD_EXPRESSION,
    CMD_IF,
    CMD_ELSE,
    CMD_WHILE,
    CMD_BEGIN,
    CMD_UNTIL,
    CMD_FOR,
    CMD_FOREACH,
    CMD_ASM
} cmd_t;

typedef struct command_t
{
    cmd_t cmd;
    expr_t *expression;
    struct command_t *next;
    struct command_t *contents;
    struct command_t *alt_contents;
} command_t;

// --== retrolang.c ==--

extern char *_out_name;
extern FILE *_out;


// --== datatype.c ==--

// Calculate datatype size
void datatype_calcsize(datatype_t *datatype);
// Add Data Type
datatype_t *datatype_add(char *name, nativetype_t nativetype, bool is_signed);
// Add Structure
datatype_t *datatype_add_structure(char *name);
// Add Field to Structure
var_t *datatype_add_field(datatype_t *structure, char *name, datatype_t *datatype, int offset, bool is_array, uint16_t array_size, bool is_pointer, uint8_t pointer_level);
// Find Data Type
datatype_t *datatype_find(char *name);
// Find Field in Structure
var_t *datatype_find_field(datatype_t *structure, char *name);

// --== error.c ==--

// Emit error
void error(char *fmt, ...);
// Emit Token error
void error_token(token_t *token, char *fmt, ...);
// Match token
void match_token(bool cmp, token_t *token, char *fmt, ...);
// Emit Expression Error
void error_expr(expr_t *e, char *fmt, ...);

// --== var.c ==--

// Find global variable
var_t *var_find_global(char *name);
// Add global variable
var_t *var_add_global(char *name, datatype_t *datatype, bool is_array, uint16_t array_size, bool is_pointer, uint8_t pointer_level);
// Calculate variable size (var size or array item size)
int32_t var_calc_item_size(var_t *v);
// Calculate total variable size (with array)
int32_t var_calc_total_size(var_t *v);
// Calculate real data type
datatype_t * var_calc_datatype(var_t *v);

// --== func.c ==--

// Get Glocal Context
func_t *func_global();
// Find function
func_t *func_find(char *name);
// Add function
func_t *func_add(char *name);
// Find local variable
var_t *func_find_var(func_t *func, char *name);
// Add local variable to function
var_t *func_add_var(func_t *func, char *name, datatype_t *datatype, bool is_array, uint16_t array_size, bool is_pointer, uint8_t pointer_level);
// Add argument to function
var_t *func_add_arg(func_t *func, char *name, datatype_t *datatype, bool is_pointer, uint8_t pointer_level);
// Get list
func_t *func_get_list();

// --== utils.c ==--

// Get filename in path
char* get_filename(char* filename);

// --== source.c ==--

// Get source list
source_t *source_get_list();
// Open source file
source_t *source_open(char *filename);
// Reset source file
void source_reset(source_t *source);
// Close source file
void source_close(source_t *source);
// Next char
char source_next_char(source_t *source);
// Get current char
char source_get_char(source_t *source);
// Get current escaped char
char source_get_escape_char(source_t *source);
// Get lower current char
char source_get_lower(source_t *source);
// Char is equal
bool source_is_equal(source_t *source, char c);
// Char is between range
bool source_is_between(source_t *source, char min, char max);
// Current char is white space
bool source_is_space(source_t *source);
// Current char is number
bool source_is_digit(source_t *source);
// Current char is symbol
bool source_is_symbol(source_t *source);
// Current char is hexadecimal
bool source_is_hexadecimal(source_t *source);
// Current char is octal
bool source_is_octal(source_t *source);
// Current char is binary
bool source_is_binary(source_t *source);
// Concat char on peek token
void source_concat_char(source_t *source);
// Concat escaped char on peek token
void source_concat_escaped_char(source_t *source);
// Concat lower char on peek token
void source_concat_lower_char(source_t *source);

// --== token.c ==--

// Scan next token
token_t *token_scan(source_t *src);
// Get current token
token_t *token_curr(source_t *src);
// Peek next token
token_t *token_peek(source_t *src);
// Verify if token is
bool token_is(source_t *src, tok_t tok);

// --== parser.c ==--

// Process source file
void parser_process_file(char *filename);

// --== parse_const_expr.c ==--

// Parse constant expression
int32_t parse_const_expr(source_t *src, command_t *cmd, func_t *func);

// --== parse_expr.c ==--

// Parse expression
expr_t *parse_expr(source_t *src, command_t *cmd, func_t *func);

// --== out.c ==--

// Emit text to output
void out_inline(char *fmt, ...);
// Emit text line to output
void out_line(char *fmt, ...);
// Emit formmated string to output
void out_inline_vargs(char *fmt, va_list args);

// --== codegen.c ==--

// Generate output code
void codegen();

// --== cpu/CPU.c ==--

// Initialize cpu
void cpu_init();
// Get compiler name extension
char *cpu_ext();
// Get format name
char *cpu_name();
// Generate global variable allocation
void cpu_global_variable(char *name, int32_t size);
// Generate function start
void cpu_function_start(char *name, int32_t vars_size);
// Generate function end
void cpu_function_end(char *name, int32_t vars_size);
// Get arguments offset
int32_t cpu_function_args_offset();
// Get variable offset
int32_t cpu_function_vars_offset();
// Set accumulator value
void cpu_set_acc(nativetype_t nt, int32_t value);
// Set aux value
void cpu_set_aux(nativetype_t nt, int32_t value);
// Store value to integer global variable
void cpu_store_global_var(nativetype_t nt, char *name);
// Store value to integer local variable
void cpu_store_local_var(nativetype_t nt, char *name, int32_t offset);
// Load acc register from integer global variable
void cpu_load_global_var(nativetype_t nt, char *name);
// Load acc register from integer local variable
void cpu_load_local_var(nativetype_t nt, char *name, int32_t offset);
// Load aux register from integer global variable
void cpu_load_aux_global_var(nativetype_t nt, char *name);
// Load aux register from integer local variable
void cpu_load_aux_local_var(nativetype_t nt, char *name, int32_t offset);
// Add values
void cpu_add(nativetype_t nt);
// Push accumulator
void cpu_push_acc(nativetype_t nt);
// Push accumulator
void cpu_pop_acc(nativetype_t nt);
// Pop aux
void cpu_push_acc(nativetype_t nt);
// Pop aux
void cpu_pop_acc(nativetype_t nt);
// Exchange accumulator with acc
void cpu_xchg_acc_aux(nativetype_t nt);
// Multiply values
void cpu_mul(nativetype_t nt, bool is_signed);
// Divide values
void cpu_div(nativetype_t nt, bool is_signed);
// Module values
void cpu_mod(nativetype_t nt, bool is_signed);
// Subtract values
void cpu_sub(nativetype_t nt);
// And values
void cpu_and(nativetype_t nt);
// Or values
void cpu_or(nativetype_t nt);
// Xor values
void cpu_xor(nativetype_t nt);
// Compare values
void cpu_compare(nativetype_t nt, tok_t operation, bool is_signed);
// Jump if true
void cpu_jump_if_true(nativetype_t nt, int label);
// Jump if false
void cpu_jump_if_false(nativetype_t nt, int label);
// Jump always
void cpu_jump(int label);
// Label
void cpu_label(int label);
// Get comment start
char *cpu_comment_start();
// Get comment end
char *cpu_comment_end();
// Convet to 8bit acc
void cpu_convert_to_8bit(nativetype_t from, bool from_signed, bool to_signed);
// Convet to 16bit acc
void cpu_convert_to_16bit(nativetype_t from, bool from_signed, bool to_signed);
// Convet to 24bit acc
void cpu_convert_to_24bit(nativetype_t from, bool from_signed, bool to_signed);
// Convet to 32bit acc
void cpu_convert_to_32bit(nativetype_t from, bool from_signed, bool to_signed);
// Set acc value from pointer address
void cpu_set_acc_as_pointer(nativetype_t nt, char *name);
// set aux value from pointer address
void cpu_set_aux_as_pointer(nativetype_t nt, char *name);