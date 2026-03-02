#include "asm.h"

void validate(expr_t *e, bool support_byte, bool support_word, bool support_dword, bool support_qword)
{
    if
    (
        (!support_byte && e->force_byte) ||
        (!support_word && e->force_word) ||
        (!support_dword && e->force_dword) ||
        (!support_qword && e->force_qword) || e->force_short || e->force_near || e->force_far
    )
    {
        error_expr(e, "size suffix not supported.");
    }
    int check = 0;
    check += e->force_byte ? 1 : 0;
    check += e->force_word ? 1 : 0;
    check += e->force_dword ? 1 : 0;
    check += e->force_qword ? 1 : 0;
    if(check > 1) error_expr(e, "multiple incompatible size suffix.");
}
void validate_distance(expr_t *e, bool support_short, bool support_near, bool support_far)
{
    if
    (
        (!support_near && e->force_byte) ||
        (!support_short && e->force_word) ||
        (!support_far && e->force_dword) ||
        e->force_byte || e->force_word || e->force_dword || e->force_qword
    )
    {
        error_expr(e, "distance suffix not supported.");
    }
    int check = 0;
    check += e->force_short ? 1 : 0;
    check += e->force_near ? 1 : 0;
    check += e->force_far ? 1 : 0;
    if(check > 1) error_expr(e, "multiple incompatible distance suffix.");
}

expr_t *filter_registers(expr_t *e)
{
    if(!e) return e;
    if(e->left) e->left = filter_registers(e->left);
    if(e->right) e->right = filter_registers(e->right);
    if(e->token == TOK_REGISTER)
    {
        e->token = TOK_VALUE;
        e->value = 0;
    }
    return e;
}

expr_t *optimize(expr_t *e)
{
    if(!e) return e;
    if(e->left) e->left = optimize(e->left);
    if(e->right) e->right = optimize(e->right);
    if(e->token == TOK_SYMBOL)
    {
        if(consts_exists(e->text))
        {
            e->value = consts_get(e->text);
            e->token = TOK_VALUE;
        }
    }
    else if(e->left && e->right && e->left->token == TOK_VALUE && e->right->token == TOK_VALUE)
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
                e->value = e->left->value ^ e->right->value;
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

bool generate(expr_t *e, int offset, bool is_seg)
{
    bool ret = false;
    if(!e) return false;
    if(e->left) ret |= generate(e->left, offset, is_seg);
    if(e->right) ret |= generate(e->right, offset, is_seg);
    switch (e->token)
    {
    case TOK_REGISTER:
        {
            out(REC_EXPR_PUSH_VALUE, 0, 0, 0, 0);
        }
        break;
    case TOK_VALUE:
        {
            if(e->value < INT16_MIN) error_expr(e, "value underflow.");
            if(e->value > UINT16_MAX) error_expr(e, "value overflow.");
            out(e->value >= INT16_MAX ? REC_EXPR_PUSH_VALUE_UNSIGNED : REC_EXPR_PUSH_VALUE, e->value, 0, 0, 0);
        }
        break;
    case TOK_SYMBOL:
        {
            out(is_seg ? REC_EXPR_PUSH_SEGMENT : REC_EXPR_PUSH_CONST, 0, 0, e->text, strlen(e->text));
            ret = true;
        }
        break;
    case TOK_CURRENT_POS:
        {
            out(REC_EXPR_PUSH_OFFSET, offset, 0, 0, 0);
            ret = true;
        }
        break;
    case TOK_ADD:
        {
            out(REC_EXPR_ADD, 0, 0, 0, 0);
        }
        break;
    case TOK_SUB:
        {
            out(REC_EXPR_SUB, 0, 0, 0, 0);
        }
        break;
    case TOK_MUL:
        {
            out(REC_EXPR_MUL, 0, 0, 0, 0);
        }
        break;
    case TOK_DIV:
        {
            out(REC_EXPR_DIV, 0, 0, 0, 0);
        }
        break;
    case TOK_MOD:
        {
            out(REC_EXPR_MOD, 0, 0, 0, 0);
        }
        break;
    
    default:
        error_expr(e, "invalid expression. [token #%i:%s]", e->token, e->text);
        break;
    }
    return ret;
}