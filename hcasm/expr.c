#include "asm.h"

// [English] Validate that the expression supports the given size prefixes.
// [Portuguese] Valida que a expressão suporta os prefixos de tamanho fornecidos.
// [English] Errors if an unsupported size prefix is used or if multiple incompatible sizes are combined.
// [Portuguese] Erro se um prefixo de tamanho não suportado for usado ou se múltiplos tamanhos incompatíveis forem combinados.
void validate(expr_t *e, bool support_byte, bool support_word, bool support_dword, bool support_qword)
{
    // [English] Check each size prefix against what's supported for this instruction
    // [Portuguese] Verifica cada prefixo de tamanho contra o que é suportado para esta instrução
    if (
        (!support_byte && e->force_byte) ||
        (!support_word && e->force_word) ||
        (!support_dword && e->force_dword) ||
        (!support_qword && e->force_qword) || e->force_short || e->force_near || e->force_far)
    {
        error_expr(e, "size suffix not supported.");
    }
    // [English] Ensure not combining multiple size prefixes (e.g., byte + word)
    // [Portuguese] Garante que não está combinando múltiplos prefixos de tamanho (ex.: byte + word)
    int check = 0;
    check += e->force_byte ? 1 : 0;
    check += e->force_word ? 1 : 0;
    check += e->force_dword ? 1 : 0;
    check += e->force_qword ? 1 : 0;
    if (check > 1)
        error_expr(e, "multiple incompatible size suffix.");
}

// [English] Validate that the expression supports the given distance prefixes (short/near/far).
// [Portuguese] Valida que a expressão suporta os prefixos de distância fornecidos (short/near/far).
// [English] Errors if an unsupported distance prefix is used or if multiple are combined.
// [Portuguese] Erro se um prefixo de distância não suportado for usado ou se múltiplos forem combinados.
void validate_distance(expr_t *e, bool support_short, bool support_near, bool support_far)
{
    if (
        (!support_short && e->force_short) ||
        (!support_near && e->force_near) ||
        (!support_far && e->force_far))
    {
        error_expr(e, "distance suffix not supported.");
    }
    int check = 0;
    check += e->force_short ? 1 : 0;
    check += e->force_near ? 1 : 0;
    check += e->force_far ? 1 : 0;
    if (check > 1)
        error_expr(e, "multiple incompatible distance suffix.");
}

// [English] Recursively walk the expression tree and replace any TOK_REGISTER nodes
// [Portuguese] Percorre recursivamente a árvore de expressão e substitui nós TOK_REGISTER
// [English] with TOK_VALUE / value=0. Used when an expression must be resolved to pure values.
// [Portuguese] por TOK_VALUE / valor=0. Usado quando uma expressão deve ser resolvida para valores puros.
expr_t *filter_registers(expr_t *e)
{
    if (!e)
        return e;
    if (e->left)
        e->left = filter_registers(e->left);
    if (e->right)
        e->right = filter_registers(e->right);
    if (e->token == TOK_REGISTER)
    {
        e->token = TOK_VALUE;
        e->value = 0;
    }
    return e;
}

// [English] Recursively constant-fold and simplify an expression tree.
// [Portuguese] Dobra constantes recursivamente e simplifica uma árvore de expressão.
// [English] Replaces symbols with constant values, evaluates low-byte/high-byte operators,
// [Portuguese] Substitui símbolos por valores constantes, avalia operadores low-byte/high-byte,
// [English] and folds constant sub-expressions (e.g., 3 + 5 -> 8).
// [Portuguese] e dobra sub-expressões constantes (ex.: 3 + 5 -> 8).
expr_t *optimize(expr_t *e)
{
    if (!e)
        return e;
    if (e->left)
        e->left = optimize(e->left);
    if (e->right)
        e->right = optimize(e->right);

    // [English] Resolve named constants (EQU / DEFINE) to their values
    // [Portuguese] Resolve constantes nomeadas (EQU / DEFINE) para seus valores
    if (e->token == TOK_SYMBOL)
    {
        if (consts_exists(e->text))
        {
            e->value = consts_get(e->text);
            e->token = TOK_VALUE;
        }
    }
    // [English] Low-byte operator (<value) -> mask to 8 bits
    // [Portuguese] Operador low-byte (<valor) -> mascara para 8 bits
    else if (e->token == TOK_LOBYTE && e->right && e->right->token == TOK_VALUE)
    {
        e->value = e->right->value & 0xFF;
        e->token = TOK_VALUE;
        free_expr(e->right);
        e->right = 0;
    }
    // [English] High-byte operator (>value) -> shift down to 8 bits
    // [Portuguese] Operador high-byte (>valor) -> desloca para 8 bits
    else if (e->token == TOK_HIBYTE && e->right && e->right->token == TOK_VALUE)
    {
        e->value = (e->right->value >> 8) & 0xFF;
        e->token = TOK_VALUE;
        free_expr(e->right);
        e->right = 0;
    }
    // [English] Fold binary operations where both operands are constant values
    // [Portuguese] Dobra operações binárias onde ambos operandos são valores constantes
    else if (e->left && e->right && e->left->token == TOK_VALUE && e->right->token == TOK_VALUE)
    {
        switch (e->token)
        {
        case TOK_ADD:
            e->value = e->left->value + e->right->value;
            break;
        case TOK_SUB:
            e->value = e->left->value - e->right->value;
            break;
        case TOK_DIV:
            e->value = e->left->value / e->right->value;
            break;
        case TOK_MUL:
            e->value = e->left->value * e->right->value;
            break;
        case TOK_MOD:
            e->value = e->left->value % e->right->value;
            break;
        default:
            return e;
        }
        e->token = TOK_VALUE;
        free_expr(e->left);
        free_expr(e->right);
        e->left = 0;
        e->right = 0;
    }
    return e;
}

// [English] Generate object code records for an expression tree.
// [Portuguese] Gera registros de código objeto para uma árvore de expressão.
// [English] Walks the tree recursively and emits RPN-like records (PUSH, ADD, SUB, etc.)
// [Portuguese] Percorre a árvore recursivamente e emite registros estilo RPN (PUSH, ADD, SUB, etc.)
// [English] for the linker to evaluate.
// [Portuguese] para o ligador avaliar.
// [English] Returns true if the expression contains relocatable references (symbols or $).
// [Portuguese] Retorna true se a expressão contém referências realocáveis (símbolos ou $).
// [English] Arguments:
// [Portuguese] Argumentos:
// [English]   e       - expression tree to generate
// [Portuguese]   e       - árvore de expressão para gerar
// [English]   offset  - byte offset from expression to current position (for $ resolution)
// [Portuguese]   offset  - deslocamento em bytes da expressão até a posição atual (para resolução de $)
// [English]   is_seg  - true if generating a segment reference
// [Portuguese]   is_seg  - true se estiver gerando uma referência de segmento
bool generate(expr_t *e, int offset, bool is_seg)
{
    bool ret = false;
    if (!e)
        return false;

    // [English] Recursively generate left and right subtrees first (post-order traversal)
    // [Portuguese] Gera recursivamente subárvores esquerda e direita primeiro (percurso pós-ordem)
    if (e->left)
        ret |= generate(e->left, offset, is_seg);
    if (e->right)
        ret |= generate(e->right, offset, is_seg);

    // [English] Emit record for this node's operation
    // [Portuguese] Emite registro para a operação deste nó
    switch (e->token)
    {
    case TOK_REGISTER:
    {
        // [English] Register reference in expression -> push zero placeholder
        // [Portuguese] Referência de registrador na expressão -> empilha zero como placeholder
        out(REC_EXPR_PUSH_VALUE, 0, 0, 0, 0);
    }
    break;
    case TOK_VALUE:
    {
        // [English] Constant value -> push with overflow/underflow check
        // [Portuguese] Valor constante -> empilha com verificação de overflow/underflow
        if (e->value > UINT16_MAX)
        {
            error_expr(e, "value overflow.");
        }
        else if (e->value < INT16_MIN)
        {
            error_expr(e, "value underflow.");
        }
        out(e->value >= INT16_MAX ? REC_EXPR_PUSH_VALUE_UNSIGNED : REC_EXPR_PUSH_VALUE, e->value, 0, 0, 0);
    }
    break;
    case TOK_SYMBOL:
    {
        // [English] Symbolic reference -> push constant name for linker resolution
        // [Portuguese] Referência simbólica -> empilha nome da constante para resolução pelo ligador
        out(is_seg ? REC_EXPR_PUSH_SEGMENT : REC_EXPR_PUSH_CONST, 0, 0, e->text, strlen(e->text));
        ret = true;
    }
    break;
    case TOK_CURRENT_POS:
    {
        // [English] Current position ($) -> push offset from expression start
        // [Portuguese] Posição atual ($) -> empilha deslocamento do início da expressão
        out(REC_EXPR_PUSH_OFFSET, offset, 0, 0, 0);
        ret = true;
    }
    break;
    case TOK_LOBYTE:
    {
        // [English] Low byte operator -> mask with 0xFF
        // [Portuguese] Operador low-byte -> mascara com 0xFF
        out(REC_EXPR_PUSH_VALUE, 0, 0xFF, 0, 0);
        out(REC_EXPR_AND, 0, 0, 0, 0);
        ret = true;
    }
    break;
    case TOK_HIBYTE:
    {
        // [English] High byte operator -> shift right by 8
        // [Portuguese] Operador high-byte -> desloca direita por 8
        out(REC_EXPR_PUSH_VALUE, 0, 8, 0, 0);
        out(REC_EXPR_SHR, 0, 0, 0, 0);
        ret = true;
    }
    break;
    case TOK_ADD:
    {
        // [English] Addition
        // [Portuguese] Adição
        out(REC_EXPR_ADD, 0, 0, 0, 0);
    }
    break;
    case TOK_SUB:
    {
        // [English] Subtraction
        // [Portuguese] Subtração
        out(REC_EXPR_SUB, 0, 0, 0, 0);
    }
    break;
    case TOK_MUL:
    {
        // [English] Multiplication
        // [Portuguese] Multiplicação
        out(REC_EXPR_MUL, 0, 0, 0, 0);
    }
    break;
    case TOK_DIV:
    {
        // [English] Division
        // [Portuguese] Divisão
        out(REC_EXPR_DIV, 0, 0, 0, 0);
    }
    break;
    case TOK_MOD:
    {
        // [English] Modulo
        // [Portuguese] Módulo
        out(REC_EXPR_MOD, 0, 0, 0, 0);
    }
    break;

    default:
        error_expr(e, "invalid expression. [token #%i:%s]", e->token, e->text);
        break;
    }
    return ret;
}
