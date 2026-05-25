#pragma once

#include "bcomp.h"

// [English] AST node operation types
// [Portuguese] Tipos de operação de nó AST
typedef enum ast_op_t
{
    // Literals / Literais
    AST_INT_LITERAL,
    AST_CHAR_LITERAL,
    AST_STRING_LITERAL,

    // Identifiers / Identificadores
    AST_IDENT,

    // Unary operators / Operadores unários
    AST_NEG,       // -x
    AST_NOT,       // ~x
    AST_LNOT,      // !x
    AST_PRE_INC,   // ++x
    AST_PRE_DEC,   // --x
    AST_POST_INC,  // x++
    AST_POST_DEC,  // x--
    AST_DEREF,     // *x
    AST_ADDR,      // &x

    // Binary operators / Operadores binários
    AST_ADD,       // +
    AST_SUB,       // -
    AST_MUL,       // *
    AST_DIV,       // /
    AST_MOD,       // %
    AST_AND,       // &
    AST_OR,        // |
    AST_XOR,       // ^
    AST_SHL,       // <<
    AST_SHR,       // >>
    AST_EQ,        // ==
    AST_NE,        // !=
    AST_LT,        // <
    AST_GT,        // >
    AST_LE,        // <=
    AST_GE,        // >=
    AST_ASSIGN,    // =
    AST_ADD_ASSIGN,    // +=
    AST_SUB_ASSIGN,    // -=
    AST_MUL_ASSIGN,    // *=
    AST_DIV_ASSIGN,    // /=
    AST_MOD_ASSIGN,    // %=
    AST_AND_ASSIGN,    // &=
    AST_OR_ASSIGN,     // |=
    AST_XOR_ASSIGN,    // ^=

    // Function call / Chamada de função
    AST_CALL,

    // Comma operator (for function arguments) / Operador vírgula (para argumentos de função)
    AST_COMMA,

    // Array subscript / Subscrito de array
    AST_INDEX,

    // Logical (short-circuit) / Lógicos (curto-circuito)
    AST_LAND,      // &&
    AST_LOR,       // ||

    // Ternary / Ternário
    AST_COND,      // a ? b : c

    // Statement wrappers / Envelopes de comando
    AST_EXPR_STMT, // expressão como comando
    AST_IF,        // if (cond) then [else alt]
    AST_WHILE,     // while (cond) body
    AST_FOR,       // for (init; cond; next) body
    AST_RETURN,    // return [expr]
    AST_BREAK,     // break
    AST_BLOCK      // { statements }
} ast_op_t;

// [English] AST node structure
// [Portuguese] Estrutura de nó AST
typedef struct ast_node_t
{
    ast_op_t op;
    int lvalue;            // 1 se é LVALUE, 0 se RVALUE
    symbol_t *sym;         // símbolo (para identificadores)
    int int_value;         // para literais inteiros/char
    char *str_value;       // para strings
    int label;             // para labels (if/while/for)

    struct ast_node_t *left;    // filho esquerdo
    struct ast_node_t *right;   // filho direito
    struct ast_node_t *cond;    // condição (if/while/for/?:)
    struct ast_node_t *then;    // then branch (if/?:)
    struct ast_node_t *else_;   // else branch (if/?:)
    struct ast_node_t *init;    // inicialização (for)
    struct ast_node_t *next;    // próximo (for increment, lista de args, statements)

    // Optimization metadata / Metadados de otimização
    int is_const;        // 1 se é constante conhecida
    int const_value;     // valor da constante se is_const=1
} ast_node_t;

// [English] AST creation functions
// [Portuguese] Funções de criação de AST
ast_node_t *ast_int(int value);
ast_node_t *ast_char(int value);
ast_node_t *ast_string(const char *value);
ast_node_t *ast_ident(symbol_t *sym);
ast_node_t *ast_unary(ast_op_t op, ast_node_t *expr);
ast_node_t *ast_binary(ast_op_t op, ast_node_t *left, ast_node_t *right);
ast_node_t *ast_call(ast_node_t *func, ast_node_t *args);
ast_node_t *ast_index(ast_node_t *array, ast_node_t *index);
ast_node_t *ast_ternary(ast_node_t *cond, ast_node_t *then_, ast_node_t *else_);
ast_node_t *ast_assign(ast_node_t *left, ast_node_t *right);

// [English] Statement AST creation
// [Portuguese] Criação de AST de comandos
ast_node_t *ast_expr_stmt(ast_node_t *expr);
ast_node_t *ast_if(ast_node_t *cond, ast_node_t *then_, ast_node_t *else_);
ast_node_t *ast_while(ast_node_t *cond, ast_node_t *body);
ast_node_t *ast_for(ast_node_t *init, ast_node_t *cond, ast_node_t *next, ast_node_t *body);
ast_node_t *ast_return(ast_node_t *expr);
ast_node_t *ast_break(int label);
ast_node_t *ast_block(ast_node_t *stmts);

// [English] AST utility functions
// [Portuguese] Funções utilitárias de AST
void ast_free(ast_node_t *node);
ast_node_t *ast_copy(ast_node_t *node);
void ast_dump(ast_node_t *node, int indent);

// [English] AST optimization
// [Portuguese] Otimização de AST
ast_node_t *ast_optimize(ast_node_t *node);

// [English] AST code generation
// [Portuguese] Geração de código a partir de AST
void ast_gen(ast_node_t *node);
void ast_gen_rvalue(ast_node_t *node);

// [English] AST evaluation (returns 1 if can be evaluated at compile time)
// [Portuguese] Avaliação de AST (retorna 1 se pode ser avaliada em tempo de compilação)
int ast_eval(ast_node_t *node, int *result);
