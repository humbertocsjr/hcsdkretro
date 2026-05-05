// [English] BSD 4-Clause License
// [Portuguese] Licença BSD 4-Cláusulas
//
// [English] Copyright (c) 2025,2026, Humberto Costa dos Santos Junior
// [Portuguese] Direitos autorais (c) 2025,2026, Humberto Costa dos Santos Junior
// [English] All rights reserved.
// [Portuguese] Todos os direitos reservados.

#pragma once

// [English] --== headers ==--
// [Portuguese] --== cabeçalhos ==--

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

// [English] --== common ==--
// [Portuguese] --== comum ==--

#define ARGV_MAX 16

// [English] Source file handle with current position tracking
// [Portuguese] Manipulador de arquivo fonte com rastreamento de posição atual
typedef struct source_t
{
    FILE *file;
    int c;
    int line;
    int column;
    char filename[1];
} source_t;

// [English] Token types produced by the scanner/lexer
// [Portuguese] Tipos de token produzidos pelo scanner/analisador léxico
typedef enum token_t
{
    TOK_EOF,            // [English] End of file
                        // [Portuguese] Fim do arquivo
    TOK_NEWLINE,        // [English] End of line
                        // [Portuguese] Fim da linha
    TOK_SYMBOL,         // [English] Identifier / symbol / label reference
                        // [Portuguese] Identificador / símbolo / referência de rótulo
    TOK_VALUE,          // [English] Numeric literal
                        // [Portuguese] Literal numérico
    TOK_COMMA,          // [English] Comma separator
                        // [Portuguese] Separador de vírgula
    TOK_COLON,          // [English] Colon (label definition)
                        // [Portuguese] Dois-pontos (definição de rótulo)
    TOK_STRING,         // [English] String literal
                        // [Portuguese] Literal de string
    TOK_REGISTER,       // [English] CPU register name
                        // [Portuguese] Nome de registrador da CPU
    TOK_MNEMONIC,       // [English] Instruction mnemonic
                        // [Portuguese] Mnemônico de instrução
    TOK_CURRENT_POS,    // [English] Current position ($)
                        // [Portuguese] Posição atual ($)
    TOK_ADD,            // [English] Addition operator (+)
                        // [Portuguese] Operador de adição (+)
    TOK_SUB,            // [English] Subtraction operator (-)
                        // [Portuguese] Operador de subtração (-)
    TOK_MUL,            // [English] Multiplication operator (*)
                        // [Portuguese] Operador de multiplicação (*)
    TOK_DIV,            // [English] Division operator (/)
                        // [Portuguese] Operador de divisão (/)
    TOK_MOD,            // [English] Modulo operator (%)
                        // [Portuguese] Operador de módulo (%)
    TOK_INDEX_OPEN,     // [English] Index open bracket ([)
                        // [Portuguese] Colchete de abertura ([)
    TOK_INDEX_CLOSE,    // [English] Index close bracket (])
                        // [Portuguese] Colchete de fechamento (])
    TOK_PARAMS_OPEN,    // [English] Parameters open (()
                        // [Portuguese] Parêntese de abertura (()
    TOK_PARAMS_CLOSE,   // [English] Parameters close ())
                        // [Portuguese] Parêntese de fechamento ())
    TOK_SUB_LABEL,      // [English] Sub-label reference (.)
                        // [Portuguese] Referência de sub-rótulo (.)
    TOK_HASH,           // [English] Immediate prefix (#)
                        // [Portuguese] Prefixo imediato (#)
    TOK_LOBYTE,         // [English] Low byte operator (<)
                        // [Portuguese] Operador de byte baixo (<)
    TOK_HIBYTE          // [English] High byte operator (>)
                        // [Portuguese] Operador de byte alto (>)
} token_t;

// [English] CPU register definition (name, numeric value, auxiliary value, group flags)
// [Portuguese] Definição de registrador da CPU (nome, valor numérico, valor auxiliar, flags de grupo)
typedef struct reg_t
{
    char *name;
    int value;
    int value_aux;
    int group;
} reg_t;

// [English] Expression tree node -- represents a parsed expression or token
// [Portuguese] Nó da árvore de expressão -- representa uma expressão ou token analisado
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
    bool immediate;
    struct expr_t *left;
    struct expr_t *right;
    char text[1];
} expr_t;

// [English] Opcode table entry -- mnemonic + up to 6 opcode bytes + emit function
// [Portuguese] Entrada da tabela de opcode -- mnemônico + até 6 bytes de opcode + função de emissão
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

// [English] Linked list node for named constants (EQU / DEFINE)
// [Portuguese] Nó de lista encadeada para constantes nomeadas (EQU / DEFINE)
typedef struct const_t
{
    int value;
    struct const_t *next;
    char name[1];
} const_t;

// [English] --== error.c ==--
// [Portuguese] --== error.c ==--

// [English] Print error message to stderr and exit
// [Portuguese] Imprime mensagem de erro no stderr e sai
void error(char *fmt, ...);
// [English] Print error with source position (file:line:col) and exit
// [Portuguese] Imprime erro com posição no fonte (arquivo:linha:coluna) e sai
void error_expr(expr_t *e, char *fmt, ...);

// [English] --== main.c ==--
// [Portuguese] --== main.c ==--

// [English] Currently active source file being parsed
// [Portuguese] Arquivo fonte atualmente ativo sendo analisado
extern source_t *_source;

// [English] --== obj.c ==--
// [Portuguese] --== obj.c ==--

// [English] Open output (.obj) and optional dump file
// [Portuguese] Abre arquivo de saída (.obj) e arquivo opcional de dump
void out_open(char *name, char *dump_name);
// [English] Close output and dump files
// [Portuguese] Fecha arquivos de saída e dump
void out_close();
// [English] Write a typed record to the object file (type, value, aux, data, size)
// [Portuguese] Escreve um registro tipado no arquivo objeto (tipo, valor, aux, dados, tamanho)
void out(rectype_t type, int16_t value, uint16_t aux, void *data, uint8_t data_size);

// [English] --== source.c ==--
// [Portuguese] --== source.c ==--

// [English] Advance to next character in source and return it
// [Portuguese] Avança para o próximo caractere no fonte e o retorna
char source_nextc();
// [English] Return current character without advancing
// [Portuguese] Retorna caractere atual sem avançar
char source_getc();
// [English] Read escape sequence (\\n, \\r, \\t, \\a, \\b) and return decoded char
// [Portuguese] Lê sequência de escape (\\n, \\r, \\t, \\a, \\b) e retorna caractere decodificado
char source_getescapec();
// [English] Open a source file for reading
// [Portuguese] Abre um arquivo fonte para leitura
void source_open(char *filename);
// [English] Close current source file and restore previous include level
// [Portuguese] Fecha arquivo fonte atual e restaura nível de inclusão anterior
void source_close();
// [English] Check if current character equals a specific value
// [Portuguese] Verifica se o caractere atual é igual a um valor específico
char source_is(char c);
// [English] Check if current character is in a range [min, max]
// [Portuguese] Verifica se o caractere atual está em um intervalo [min, max]
char source_between(char min, char max);

// [English] --== parser.c ==--
// [Portuguese] --== parser.c ==--

// [English] Current label being defined
// [Portuguese] Rótulo atual sendo definido
extern char *_current_label;
// [English] Parse a source file (top-level entry point)
// [Portuguese] Analisa um arquivo fonte (ponto de entrada principal)
void parse(char *filename);

// [English] --== scanner.c ==--
// [Portuguese] --== scanner.c ==--

// [English] Scan and return the next token from source
// [Portuguese] Escaneia e retorna o próximo token do fonte
expr_t *scan();
// [English] Return the current token without scanning
// [Portuguese] Retorna o token atual sem escanear
expr_t *curr();
// [English] Check if current token matches a specific type
// [Portuguese] Verifica se o token atual corresponde a um tipo específico
bool curr_is(token_t token);
// [English] Check if the next token matches a specific type
// [Portuguese] Verifica se o próximo token corresponde a um tipo específico
bool next_is(token_t token);
// [English] Deep-clone an expression tree node
// [Portuguese] Clona profundamente um nó da árvore de expressão
expr_t *clone_expr(expr_t *src);
// [English] Free an expression tree recursively
// [Portuguese] Libera uma árvore de expressão recursivamente
void free_expr(expr_t *e);
// [English] Check if current token text matches a keyword
// [Portuguese] Verifica se o texto do token atual corresponde a uma palavra-chave
bool curr_is_keyword(char *keyword);
// [English] Check if a token text matches a keyword
// [Portuguese] Verifica se um texto de token corresponde a uma palavra-chave
bool is_keyword(char *token, char *keyword);

// [English] --== cpu/?????.c ==--
// [Portuguese] --== cpu/?????.c ==--

// [English] CPU architecture type identifier
// [Portuguese] Identificador do tipo de arquitetura da CPU
extern rectype_t _cpu;
// [English] CPU register table (null-terminated)
// [Portuguese] Tabela de registradores da CPU (terminada em nulo)
extern reg_t _regs[];
// [English] CPU prefix opcode table (null-terminated)
// [Portuguese] Tabela de opcodes de prefixo da CPU (terminada em nulo)
extern opcode_t _prefix[];
// [English] CPU instruction opcode table (null-terminated)
// [Portuguese] Tabela de opcodes de instrução da CPU (terminada em nulo)
extern opcode_t _opcode[];

// [English] --== expr.c ==--
// [Portuguese] --== expr.c ==--

// [English] Remove register references from expression, converting to value-only tree
// [Portuguese] Remove referências de registradores da expressão, convertendo para árvore apenas de valores
expr_t *filter_registers(expr_t *e);
// [English] Validate that the expression supports a given size prefix (byte/word/dword/qword)
// [Portuguese] Valida que a expressão suporta um determinado prefixo de tamanho (byte/word/dword/qword)
void validate(expr_t *e, bool support_byte, bool support_word, bool support_dword, bool support_qword);
// [English] Validate that the expression supports a given distance prefix (short/near/far)
// [Portuguese] Valida que a expressão suporta um determinado prefixo de distância (short/near/far)
void validate_distance(expr_t *e, bool support_short, bool support_near, bool support_far);
// [English] Constant-fold and simplify an expression tree
// [Portuguese] Dobra constantes e simplifica uma árvore de expressão
expr_t *optimize(expr_t *e);
// [English] Generate object code for an expression (offset = byte distance from expression to current position)
// [Portuguese] Gera código objeto para uma expressão (offset = distância em bytes da expressão até a posição atual)
bool generate(expr_t *e, int offset, bool is_seg);

// [English] --== consts.c ==--
// [Portuguese] --== consts.c ==--

// [English] Check if a named constant exists
// [Portuguese] Verifica se uma constante nomeada existe
bool consts_exists(char *name);
// [English] Get the value of a named constant
// [Portuguese] Obtém o valor de uma constante nomeada
int consts_get(char *name);
// [English] Set or create a named constant with a given value
// [Portuguese] Define ou cria uma constante nomeada com um determinado valor
void consts_set(char *name, int value);
