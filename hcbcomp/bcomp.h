#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

// --== Token types ==--

typedef enum token_t
{
    TOK_EOF = 0,
    TOK_IDENT,
    TOK_NUMBER,
    TOK_STRING,
    TOK_CHAR,
    TOK_IF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_FOR,
    TOK_DO,
    TOK_RETURN,
    TOK_BREAK,
    TOK_AUTO,
    TOK_EXTRN,
    TOK_ASM,
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_PERCENT,
    TOK_AMPERSAND,
    TOK_PIPE,
    TOK_CARET,
    TOK_TILDE,
    TOK_BANG,
    TOK_LT,
    TOK_GT,
    TOK_EQ,
    TOK_NE,
    TOK_LE,
    TOK_GE,
    TOK_LSHIFT,
    TOK_RSHIFT,
    TOK_AND,
    TOK_OR,
    TOK_INC,
    TOK_DEC,
    TOK_ASSIGN,
    TOK_ADD_ASSIGN,
    TOK_SUB_ASSIGN,
    TOK_MUL_ASSIGN,
    TOK_DIV_ASSIGN,
    TOK_MOD_ASSIGN,
    TOK_AND_ASSIGN,
    TOK_OR_ASSIGN,
    TOK_XOR_ASSIGN,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACKET,
    TOK_RBRACKET,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_SEMICOLON,
    TOK_COMMA,
    TOK_COLON,
    TOK_DOT,
} token_t;

// --== Symbol table ==--

typedef enum symkind_t
{
    SYM_GLOBAL,
    SYM_LOCAL,
    SYM_PARAM,
    SYM_EXTERN,
    SYM_FUNCTION
} symkind_t;

typedef struct symbol_t
{
    char *name;
    symkind_t kind;
    int offset; // stack offset (for locals/params) or address (for globals)
    int size;   // array size (0 for scalars)
    struct symbol_t *next;
} symbol_t;

// --== Expression kinds ==--

#define VAL_LVALUE 1
#define VAL_RVALUE 0

// --== Globals ==--

extern FILE *outfile;
extern const char *target_cpu;
extern int line_num;
extern int col_num;
extern const char *filename;

// --== Lexer ==--

extern token_t tok;
extern char tok_text[256];
extern int tok_value;

void lex_init(FILE *fp);
void next(void);
void next_token(void);

// --== Parser ==--

void compile_unit(void);
void statement(void);
void compound_statement(void);
void declaration(void);
int expression(void);
int assignment(void);
int conditional(void);

// --== Symbol table ==--

symbol_t *lookup(const char *name);
symbol_t *install(const char *name, symkind_t kind, int size);

// --== Preprocessor ==--

void preproc_run(const char *filename, FILE *output, const char *cpu);
void preproc_add_define(const char *name, const char *value);
void preproc_add_include_dir(const char *dir);

// --== Error ==--

void error(const char *fmt, ...);

// --== Code generation ==--

int gen_label(void);
void gen_text(void);
void gen_data(void);
void gen_global(const char *name);
void gen_extern(const char *name);
void gen_label_str(const char *name);
void gen_label_int(int label);
void gen_word(int val);
void gen_bytes(const char *str);
void gen_reserve(int n);
void gen_comment(const char *fmt, ...);
void gen_emit_raw(const char *line);
void gen_data_final(void);

void gen_prologue(const char *name, int nlocals);
void gen_epilogue(void);
void gen_return(void);

void gen_push_prim(void);
void gen_pop_sec(void);

void gen_load_imm(int val);
void gen_load_label(int label);
void gen_load_var(const char *name);
void gen_load_addr(const char *name);
void gen_store_local(int offset);
void gen_load_local(int offset);
void gen_local_addr(int offset);
void gen_store_param(int offset);
void gen_load_param(int offset);
void gen_param_addr(int offset);
void gen_store_global(const char *name);
void gen_load_global(const char *name);
void gen_deref(void);
void gen_peekb(void);
void gen_pokeb(void);
void gen_store_to_addr(void);

void gen_add(void);
void gen_double(void);
void gen_sub(void);
void gen_mul(void);
void gen_div(void);
void gen_mod(void);

void gen_neg(void);
void gen_not(void);
void gen_lnot(void);

void gen_and(void);
void gen_or(void);
void gen_xor(void);
void gen_shl(void);
void gen_shr(void);

void gen_cmp_eq(void);
void gen_cmp_ne(void);
void gen_cmp_lt(void);
void gen_cmp_gt(void);
void gen_cmp_le(void);
void gen_cmp_ge(void);

void gen_jmp(int label);
void gen_jz(int label);
void gen_jnz(int label);

void gen_call(const char *name, int nargs);
void gen_reverse_args(int count);
