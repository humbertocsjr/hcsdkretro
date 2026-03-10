#include "../asm.h"

rectype_t _cpu = REC_CPU_Z80;

enum
{
    REG_8BIT = 0x01,
    REG_16BIT = 0x02,
    REG_16BIT_SP = 0x04,
    REG_16BIT_AF = 0x08,
    REG_MAIN_PTR = 0x10,
    REG_OTHER_PTR = 0x20,
    REG_IX_PTR = 0x40,
    REG_IY_PTR = 0x80,
    REG_CMP = 0x100,
    REG_16BIT_ALT = 0x200,
    REG_A = 0x400,
    REG_HL = 0x800,
    REG_SP = 0x1000,
    REG_I = 0x2000,
    REG_R = 0x4000
};

static reg_t *tryget_reg8bit(expr_t *arg, bool include_main_ptr)
{
    if(arg->token == TOK_REGISTER && (arg->reg->group & REG_8BIT)) return arg->reg;
    if(include_main_ptr && arg->token == TOK_INDEX_OPEN && arg->right->token == TOK_REGISTER && (arg->right->reg->group & REG_MAIN_PTR)) return arg->right->reg;
    if(include_main_ptr && arg->token == TOK_INDEX_OPEN && arg->right->token == TOK_ADD && arg->right->left && arg->right->left->token == TOK_REGISTER && (arg->right->left->reg->group & REG_MAIN_PTR)) return arg->right->left->reg;
    return NULL;
}

static reg_t *tryget_reg(expr_t *arg, int group)
{
    if(arg->token == TOK_REGISTER && (arg->reg->group & group)) return arg->reg;
    if(arg->token == TOK_INDEX_OPEN && arg->right->token == TOK_REGISTER && (arg->right->reg->group & group)) return arg->right->reg;
    if(arg->token == TOK_INDEX_OPEN && arg->right->token == TOK_ADD && arg->right->left && arg->right->left->token == TOK_REGISTER && (arg->right->left->reg->group & group)) return arg->right->left->reg;
    return NULL;
}

static bool is_other_pointer(expr_t *e)
{
    if(e->token != TOK_INDEX_OPEN) return false;
    if(e->right->token != TOK_REGISTER) return false;
    return e->right->reg->group & REG_OTHER_PTR;
}

static bool is_main_pointer(expr_t *e)
{
    if(e->token != TOK_INDEX_OPEN) return false;
    if(e->right->token == TOK_REGISTER && (e->right->reg->group & REG_MAIN_PTR)) return true;
    if(e->right->token == TOK_ADD && e->right->right->token == TOK_REGISTER && (e->right->right->reg->group & REG_MAIN_PTR)) return true;
    return false;
}

static bool is_a_register(expr_t *e)
{
    return e->token == TOK_REGISTER && (e->reg->group & REG_8BIT) && e->reg->value == 7;
}

static bool is_hl_ix_iy_register(expr_t *e)
{
    return e->token == TOK_REGISTER && (e->reg->group & REG_16BIT) && e->reg->value == 6;
}

static bool is_8bit_register(expr_t *e)
{
    return e->token == TOK_REGISTER && (e->reg->group & REG_8BIT);
}

static bool is_16bitsp_register(expr_t *e)
{
    return e->token == TOK_REGISTER && (e->reg->group & REG_16BIT_SP);
}

static bool is_register(expr_t *e, int group)
{
    return e->token == TOK_REGISTER && (e->reg->group & group);
}

static bool is_value_only(expr_t *e)
{
    switch (e->token)
    {
        case TOK_VALUE: return true;
        case TOK_SYMBOL: return true;
        case TOK_SUB_LABEL: return true;
        case TOK_CURRENT_POS: return true;
        case TOK_MNEMONIC: return false;
        case TOK_REGISTER: return false;
        case TOK_INDEX_OPEN: return false;
        default: 
            if(e->left && !is_value_only(e->left)) return false;
            if(e->right && !is_value_only(e->right)) return false;
            return true;
    }
}

static bool is_address_only(expr_t *e)
{
    if(e->token != TOK_INDEX_OPEN) return false;
    return is_value_only(e->right);
}

static void emit_simple(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    if(argc != 0) error_expr(mnemonic, "invalid argument count.");
    out(REC_DATA, 0, 0, &opcode->op1, 1);
}

static void emit_simple_2bytes(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    if(argc != 0) error_expr(mnemonic, "invalid argument count.");
    out(REC_DATA, 0, 0, &opcode->op1, 1);
    out(REC_DATA, 0, 0, &opcode->op2, 1);
}

static void emit_logic(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op;
    reg_t *reg1 = NULL;
    reg_t *reg2 = NULL;
    expr_t *arg = NULL;
    bool is_8bit_op = false;
    bool is_16bit_op = false;
    bool is_8bit_value = false;
    if(argc == 2) 
    {
        if((reg1 = tryget_reg8bit(argv[0], true)) != NULL)
        {
            if((reg2 = tryget_reg8bit(argv[1], true)) != NULL)
            {
                arg = argv[1];
                is_8bit_op = true;
            }
            else
            {
                is_8bit_value = true;
                reg2 = NULL;
                arg = argv[1];
            }
        }
        else if((reg1 = tryget_reg(argv[0], REG_MAIN_PTR)) != NULL)
        {
            reg2 = tryget_reg(argv[1], REG_16BIT_SP);
            arg = argv[1];
            is_16bit_op = true;
        }

    }
    else if(argc == 1) 
    {
        if((reg2 = tryget_reg8bit(argv[0], true)) != NULL)
        {
            reg1 = &_regs[7];
            arg = argv[0];
            is_8bit_op = true;
        }
        else if((reg2 = tryget_reg(argv[0], REG_16BIT_SP)) != NULL)
        {
            reg2 = &_regs[6];
            arg = argv[1];
            is_16bit_op = true;
        }
        else 
        {
            arg = argv[0];
            is_8bit_value = true;
        }
    }
    else error_expr(mnemonic, "invalid argument count.");
    if(is_8bit_op && reg1 && reg2 && opcode->op1)
    {
        if(reg2->group & REG_IX_PTR)
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        else if(reg2->group & REG_IY_PTR)
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        if(opcode->op5)
        {
            op = opcode->op5;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = opcode->op1 | reg2->value;
        out(REC_DATA, 0, 0, &op, 1);
        if((reg2->group & REG_IX_PTR) || (reg2->group & REG_IY_PTR))
        {
            arg = filter_registers(arg);
            if(generate(arg->right, 2, false))
            {
                out(REC_EXPR_PUSH_OFFSET, 2, 0, 0, 0);
                out(REC_EXPR_SUB, 0, 0, 0, 0);
            }
            out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
        }
    }
    else if(is_16bit_op && reg1 && reg2 && opcode->op2)
    {
        if(opcode->op4)
        {
            op = opcode->op4;
            out(REC_DATA, 0, 0, &op, 1);
        }
        if(reg2->group & REG_IX_PTR)
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        else if(reg2->group & REG_IY_PTR)
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = opcode->op2 | reg2->value_aux << 4;
        out(REC_DATA, 0, 0, &op, 1);
    }
    else if(is_8bit_value && reg1 && reg1->value == 7 && opcode->op3)
    {
        op = opcode->op3;
        out(REC_DATA, 0, 0, &op, 1);
        generate(arg, 0, false);
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
    }
    else if(is_8bit_value && !reg1 && opcode->op3)
    {
        op = opcode->op3;
        out(REC_DATA, 0, 0, &op, 1);
        generate(arg, 0, false);
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
    }
    else error_expr(argv[0], "invalid arguments");
}

static void emit_offset8bit(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    if(argc != 1) error_expr(mnemonic, "invalid argument count.");
    out(REC_DATA, 0, 0, &opcode->op1, 1);
    if(generate(argv[0], 0, false))
    {
        out(REC_EXPR_PUSH_OFFSET, 1, 0, 0, 0);
        out(REC_EXPR_SUB, 0, 0, 0, 0);
    }
    out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
}

static void emit_cmp__offset8bit(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op = opcode->op1;
    if(argc == 2) 
    {
        reg_t *cmp = tryget_reg(argv[0], REG_CMP);
        if(!cmp) error_expr(argv[0], "flag expected.");
        op |= cmp->value_aux << 3;
        out(REC_DATA, 0, 0, &op, 1);
        if(generate(argv[1], 0, false))
        {
            out(REC_EXPR_PUSH_OFFSET, 1, 0, 0, 0);
            out(REC_EXPR_SUB, 0, 0, 0, 0);
        }
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
    }
    else if(argc == 1 && opcode->op2)
    {
        op = opcode->op2;
        out(REC_DATA, 0, 0, &op, 1);
        if(generate(argv[0], 0, false))
        {
            out(REC_EXPR_PUSH_OFFSET, 1, 0, 0, 0);
            out(REC_EXPR_SUB, 0, 0, 0, 0);
        }
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
    }
    else error_expr(mnemonic, "invalid argument count.");
}

static void emit_cmp__offset16bit(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op = opcode->op1;
    if(argc == 2) 
    {
        reg_t *cmp = tryget_reg(argv[0], REG_CMP);
        if(!cmp) error_expr(argv[0], "flag expected.");
        op |= cmp->value_aux << 3;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[1], -1, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    else if(argc == 1 && opcode->op2)
    {
        op = opcode->op2;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[0], -1, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    else error_expr(mnemonic, "invalid argument count.");
}

static void emit_cmp(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op = opcode->op1;
    if(argc == 1) 
    {
        reg_t *cmp = tryget_reg(argv[0], REG_CMP);
        if(!cmp) error_expr(argv[0], "flag expected.");
        op |= cmp->value_aux << 3;
        out(REC_DATA, 0, 0, &op, 1);
    }
    else if(argc == 0 && opcode->op2)
    {
        op = opcode->op2;
        out(REC_DATA, 0, 0, &op, 1);
    }
    else error_expr(mnemonic, "invalid argument count.");
}

static void emit_reg16bitaf_or_reg8bit_or_index(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op;
    if(argc != 1) error_expr(mnemonic, "invalid argument count.");
    reg_t *reg = tryget_reg(argv[0], REG_16BIT_AF);
    if(reg && argv[0]->token != TOK_INDEX_OPEN && opcode->op1)
    {
        if(reg->group & REG_IX_PTR)
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        else if(reg->group & REG_IY_PTR)
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = opcode->op1 | reg->value_aux << 4;
        out(REC_DATA, 0, 0, &op, 1);
        return;
    }
    reg = tryget_reg8bit(argv[0], true);
    if(reg && opcode->op2)
    {
        if(reg->group & REG_IX_PTR)
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        else if(reg->group & REG_IY_PTR)
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = opcode->op2 | reg->value << 3;
        out(REC_DATA, 0, 0, &op, 1);
        if((reg->group & REG_IX_PTR) || (reg->group & REG_IY_PTR))
        {
            argv[0] = filter_registers(argv[0]);
            if(generate(argv[0]->right, 2, false))
            {
                out(REC_EXPR_PUSH_OFFSET, 2, 0, 0, 0);
                out(REC_EXPR_SUB, 0, 0, 0, 0);
            }
            out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
        }
        return;
    }
    error_expr(mnemonic, "invalid arguments.");
}

static void emit_rst(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op;
    if(argc != 1) error_expr(mnemonic, "invalid argument count.");
    if(argv[0]->token != TOK_VALUE) error_expr(argv[0], "value expected.");
    if((argv[0]->value & 0x38) != argv[0]->value) error_expr(argv[0], "invalid rst value.");
    op = opcode->op1 | argv[0]->value;
    out(REC_DATA, 0, 0, &op, 1);

}

static void emit_ex(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op;
    if(argc != 2) error_expr(mnemonic, "invalid argument count.");
    reg_t *reg1 = argv[0]->token == TOK_REGISTER? argv[0]->reg : NULL;
    reg_t *reg1idx = argv[0]->token == TOK_INDEX_OPEN && argv[0]->right->token == TOK_REGISTER? argv[0]->right->reg : NULL;
    reg_t *reg2 = argv[1]->token == TOK_REGISTER? argv[1]->reg : NULL;
    reg_t *reg2idx = argv[1]->token == TOK_INDEX_OPEN && argv[1]->right->token == TOK_REGISTER? argv[1]->right->reg : NULL;
    if(reg1 && reg2 && (reg1->group & REG_16BIT_AF) && reg1->value == 3 && (reg2->group & REG_16BIT_ALT) && reg2->value == 0)
    {
        out(REC_DATA, 0, 0, &opcode->op1, 1);
    }
    else if(reg1 && reg2 && (reg1->group & REG_16BIT_AF) && reg1->value == 3 && (reg2->group & REG_16BIT_AF) && reg2->value == 3)
    {
        out(REC_DATA, 0, 0, &opcode->op1, 1);
    }
    else if(reg1idx && reg2 && (reg1idx->group & REG_16BIT_SP) && reg1idx->value == 3 && (reg2->group & REG_16BIT_AF) && (reg2->group & (REG_IX_PTR | REG_IY_PTR)) == 0 && reg2->value == 6)
    {
        out(REC_DATA, 0, 0, &opcode->op2, 1);
    }
    else if(reg2idx && reg1 && (reg2idx->group & REG_16BIT_SP) && reg2idx->value == 3 && (reg1->group & REG_16BIT_AF) && (reg1->group & (REG_IX_PTR | REG_IY_PTR)) == 0 && reg1->value == 6)
    {
        out(REC_DATA, 0, 0, &opcode->op2, 1);
    }
    else if(reg1 && reg2 && (reg1->group & REG_16BIT_AF) && reg1->value == 1 && (reg2->group & REG_16BIT_AF) && (reg2->group & (REG_IX_PTR | REG_IY_PTR)) == 0 && reg2->value == 6)
    {
        out(REC_DATA, 0, 0, &opcode->op3, 1);
    }
    else if(reg1 && reg2 && (reg2->group & REG_16BIT_AF) && reg2->value == 1 && (reg1->group & REG_16BIT_AF) && (reg1->group & (REG_IX_PTR | REG_IY_PTR)) == 0 && reg1->value == 6)
    {
        out(REC_DATA, 0, 0, &opcode->op3, 1);
    }
    else error_expr(argv[0], "invalid arguments.");
}

static void emit_ld(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op;
    reg_t *reg1;
    reg_t *reg2;
    if(argc != 2) error_expr(mnemonic, "invalid argument count.");
    if((reg1 = tryget_reg8bit(argv[0], true)) && (reg2 = tryget_reg8bit(argv[1], true)))
    {
        if((reg1->group & REG_MAIN_PTR) && (reg2->group & REG_MAIN_PTR))
        {
            error_expr(argv[0], "invalid argument combination.");
        }
        if((reg1->group & (REG_IX_PTR)) || (reg2->group & (REG_IX_PTR)))
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        if((reg1->group & (REG_IY_PTR)) || (reg2->group & (REG_IY_PTR)))
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = 0x40 | (reg1->value << 3) | reg2->value;
        out(REC_DATA, 0, 0, &op, 1);
        if((reg1->group & (REG_IX_PTR | REG_IY_PTR)))
        {
            argv[0] = filter_registers(argv[0]);
            if(generate(argv[0]->right, 2, false))
            {
                out(REC_EXPR_PUSH_OFFSET, 2, 0, 0, 0);
                out(REC_EXPR_SUB, 0, 0, 0, 0);
            }
            out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
        }
        if((reg2->group & (REG_IX_PTR | REG_IY_PTR)))
        {
            argv[1] = filter_registers(argv[1]);
            if(generate(argv[1]->right, 2, false))
            {
                out(REC_EXPR_PUSH_OFFSET, 2, 0, 0, 0);
                out(REC_EXPR_SUB, 0, 0, 0, 0);
            }
            out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
        }
    }
    else if((reg1 = tryget_reg8bit(argv[0], true)) && is_value_only(argv[1]))
    {
        if((reg1->group & (REG_IX_PTR)))
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        if((reg1->group & (REG_IY_PTR)))
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = 0x06 | (reg1->value << 3);
        out(REC_DATA, 0, 0, &op, 1);
        if((reg1->group & (REG_IX_PTR | REG_IY_PTR)))
        {
            argv[0] = filter_registers(argv[0]);
            if(generate(argv[0]->right, 2, false))
            {
                out(REC_EXPR_PUSH_OFFSET, 2, 0, 0, 0);
                out(REC_EXPR_SUB, 0, 0, 0, 0);
            }
            out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
        }
        generate(argv[1], 1, false);
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
    }
    else if(is_other_pointer(argv[0]) && is_a_register(argv[1]) && (reg1 = tryget_reg(argv[0], REG_OTHER_PTR)))
    {
        op = 0x02 | reg1->value << 4;
        out(REC_DATA, 0, 0, &op, 1);
    }
    else if(is_other_pointer(argv[1]) && is_a_register(argv[0]) && (reg2 = tryget_reg(argv[1], REG_OTHER_PTR)))
    {
        op = 0x0a | reg2->value << 4;
        out(REC_DATA, 0, 0, &op, 1);
    }
    else if(is_16bitsp_register(argv[0]) && is_value_only(argv[1]) && (reg1 = tryget_reg(argv[0], REG_16BIT_SP)))
    {
        op = 0x01 | reg1->value_aux << 4;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[1], 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    else if(is_address_only(argv[0]) && is_hl_ix_iy_register(argv[1]) && (reg1 = tryget_reg(argv[1], REG_MAIN_PTR)))
    {
        if((reg1->group & (REG_IX_PTR)))
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        if((reg1->group & (REG_IY_PTR)))
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = 0x22;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[0]->right, 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    else if(is_address_only(argv[0]) && is_a_register(argv[1]) && (reg1 = tryget_reg(argv[1], REG_8BIT)))
    {
        op = 0x32;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[0]->right, 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    else if(is_address_only(argv[1]) && is_hl_ix_iy_register(argv[0]) && (reg1 = tryget_reg(argv[0], REG_MAIN_PTR)))
    {
        if((reg1->group & (REG_IX_PTR)))
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        if((reg1->group & (REG_IY_PTR)))
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = 0x2a;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[1]->right, 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    else if(is_address_only(argv[1]) && is_a_register(argv[0]) && (reg1 = tryget_reg(argv[0], REG_8BIT)))
    {
        op = 0x3a;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[1]->right, 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    else if(is_register(argv[0], REG_SP) && is_register(argv[1], REG_MAIN_PTR) && (reg1 = tryget_reg(argv[1], REG_MAIN_PTR)))
    {
        if((reg1->group & (REG_IX_PTR)))
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        if((reg1->group & (REG_IY_PTR)))
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = 0xf9;
        out(REC_DATA, 0, 0, &op, 1);
    }
    else if(is_address_only(argv[0]) && is_16bitsp_register(argv[1]) && (reg1 = tryget_reg(argv[1], REG_16BIT_SP)))
    {
        op = 0xed;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0x43 | (reg1->value_aux << 4);
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[0]->right, 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    else if(is_address_only(argv[1]) && is_16bitsp_register(argv[0]) && (reg1 = tryget_reg(argv[0], REG_16BIT_SP)))
    {
        op = 0xed;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0x4b | (reg1->value_aux << 4);
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[1]->right, 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    else if(is_a_register(argv[0]) && is_register(argv[1], REG_I))
    {
        op = 0xed;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0x57;
        out(REC_DATA, 0, 0, &op, 1);
    }
    else if(is_a_register(argv[1]) && is_register(argv[0], REG_I))
    {
        op = 0xed;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0x47;
        out(REC_DATA, 0, 0, &op, 1);
    }
    else if(is_a_register(argv[0]) && is_register(argv[1], REG_R))
    {
        op = 0xed;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0x5f;
        out(REC_DATA, 0, 0, &op, 1);
    }
    else if(is_a_register(argv[1]) && is_register(argv[0], REG_R))
    {
        op = 0xed;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0x4f;
        out(REC_DATA, 0, 0, &op, 1);
    }
    else error_expr(argv[0], "invalid destination/source combination.");
}

static void emit_bit(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op;
    reg_t *reg;
    if(argc != 2) error_expr(mnemonic, "invalid argument count.");
    if(is_value_only(argv[0]) && (reg = tryget_reg8bit(argv[1], true)))
    {
        if(argv[0]->token != TOK_VALUE) error_expr(argv[0], "constant expression expected.");
        op = 0xcb;
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op1 | argv[0]->value << 3 | argv[1]->reg->value;
        out(REC_DATA, 0, 0, &op, 1);
    }
    else error_expr(argv[0], "invalid bit/register combination.");
}

static void emit_im(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op;
    uint8_t ops[3];
    ops[0] = opcode->op1;
    ops[1] = opcode->op2;
    ops[2] = opcode->op3;
    reg_t *reg;
    if(argc != 1) error_expr(mnemonic, "invalid argument count.");
    if(is_value_only(argv[0]) && argv[0]->token != TOK_VALUE && argv[0]->value < 3)
    {
        op = opcode->op4;
        out(REC_DATA, 0, 0, &op, 1);
        op = ops[argv[0]->value];
        out(REC_DATA, 0, 0, &op, 1);
    }
    else error_expr(argv[0], "invalid constant expression.");
}

reg_t _regs[] = 
{
    {"b", 0, 0, REG_8BIT},
    {"c", 1, 3, REG_8BIT | REG_CMP},
    {"d", 2, 0, REG_8BIT},
    {"e", 3, 0, REG_8BIT},
    {"h", 4, 0, REG_8BIT},
    {"l", 5, 0, REG_8BIT},
    {"hl", 6, 2, REG_16BIT | REG_16BIT_AF | REG_16BIT_SP | REG_MAIN_PTR | REG_HL},
    {"a", 7, 0, REG_8BIT | REG_A},

    {"bc", 0, 0, REG_16BIT | REG_16BIT_AF | REG_16BIT_SP | REG_OTHER_PTR},
    {"de", 1, 1, REG_16BIT | REG_16BIT_AF | REG_16BIT_SP | REG_OTHER_PTR},
    {"ix", 6, 2, REG_16BIT | REG_16BIT_AF | REG_16BIT_SP | REG_MAIN_PTR | REG_IX_PTR},
    {"iy", 6, 2, REG_16BIT | REG_16BIT_AF | REG_16BIT_SP | REG_MAIN_PTR | REG_IY_PTR},
    {"sp", 3, 3, REG_16BIT_SP | REG_SP},
    {"af", 3, 3, REG_16BIT_AF},
    
    {"nz", 0, 0, REG_CMP},
    {"z",  1, 1, REG_CMP},
    {"nc", 2, 2, REG_CMP},
    //{"c",  3, 3, REG_CMP},
    {"po", 4, 4, REG_CMP},
    {"pe", 5, 5, REG_CMP},
    {"p",  6, 6, REG_CMP},
    {"m",  7, 7, REG_CMP},

    {"af'", 0, 0, REG_16BIT_ALT},

    {"i", 0, 0, REG_I},

    {"r", 0, 0, REG_R},

    {NULL, 0, 0, 0}
};

opcode_t _prefix[] =
{
    {"cb",      0xcb, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"dd",      0xdd, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"ddcb",    0xdd, 0xcb, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"ed",      0xed, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"fd",      0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"fdcb",    0xfd, 0xcb, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {NULL,      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, NULL}
};

opcode_t _opcode[] =
{
    {"nop",     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"djnz",    0x10, 0x00, 0x00, 0x00, 0x00, 0x00, emit_offset8bit},
    {"jr",      0x20, 0x18, 0x00, 0x00, 0x00, 0x00, emit_cmp__offset8bit},
    {"inc",     0x03, 0x04, 0x00, 0x00, 0x00, 0x00, emit_reg16bitaf_or_reg8bit_or_index},
    {"dec",     0x0b, 0x05, 0x00, 0x00, 0x00, 0x00, emit_reg16bitaf_or_reg8bit_or_index},
    {"rlca",    0x07, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"rla",     0x17, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"daa",     0x27, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"scf",     0x37, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"rrca",    0x07, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"rra",     0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"cpl",     0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"ccf",     0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},

    {"ld",      0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, emit_ld},
    {"halt",    0x76, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"add",     0x80, 0x09, 0xc6, 0x00, 0x00, 0x00, emit_logic},
    {"adc",     0x88, 0x4a, 0xce, 0xed, 0x00, 0x00, emit_logic},
    {"sub",     0x90, 0x00, 0xd6, 0x00, 0x00, 0x00, emit_logic},
    {"sbc",     0x98, 0x42, 0xde, 0xed, 0x00, 0x00, emit_logic},
    {"and",     0xa0, 0x00, 0xe6, 0x00, 0x00, 0x00, emit_logic},
    {"xor",     0xa8, 0x00, 0xee, 0x00, 0x00, 0x00, emit_logic},
    {"or",      0xb0, 0x00, 0xf6, 0x00, 0x00, 0x00, emit_logic},
    {"cp",      0xb8, 0x00, 0xfe, 0x00, 0x00, 0x00, emit_logic},

    {"ret",     0xc0, 0xc9, 0x00, 0x00, 0x00, 0x00, emit_cmp},
    {"pop",     0xc1, 0x00, 0x00, 0x00, 0x00, 0x00, emit_reg16bitaf_or_reg8bit_or_index},
    {"push",    0xc5, 0x00, 0x00, 0x00, 0x00, 0x00, emit_reg16bitaf_or_reg8bit_or_index},
    {"jp",      0xc2, 0xc3, 0x00, 0x00, 0x00, 0x00, emit_cmp__offset16bit},
    {"call",    0xc4, 0xcd, 0x00, 0x00, 0x00, 0x00, emit_cmp__offset16bit},
    {"rst",     0xc7, 0x00, 0x00, 0x00, 0x00, 0x00, emit_rst},
    {"ex",      0x08, 0xe3, 0xeb, 0x00, 0x00, 0x00, emit_ex},
    {"di",      0xf3, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"ei",      0xfb, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"exx",     0xd9, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},

    {"rlc",     0x00, 0x00, 0x00, 0x00, 0xcb, 0x00, emit_logic},
    {"rrc",     0x08, 0x00, 0x00, 0x00, 0xcb, 0x00, emit_logic},
    {"rl",      0x10, 0x00, 0x00, 0x00, 0xcb, 0x00, emit_logic},
    {"rr",      0x18, 0x00, 0x00, 0x00, 0xcb, 0x00, emit_logic},
    {"sla",     0x20, 0x00, 0x00, 0x00, 0xcb, 0x00, emit_logic},
    {"sra",     0x28, 0x00, 0x00, 0x00, 0xcb, 0x00, emit_logic},
    {"sll",     0x30, 0x00, 0x00, 0x00, 0xcb, 0x00, emit_logic},
    {"srl",     0x38, 0x00, 0x00, 0x00, 0xcb, 0x00, emit_logic},
    {"rlc",     0x00, 0x00, 0x00, 0x00, 0xcb, 0x00, emit_logic},
    {"rrc",     0x08, 0x00, 0x00, 0x00, 0xcb, 0x00, emit_logic},
    {"bit",     0x40, 0x00, 0x00, 0x00, 0x00, 0x00, emit_bit},
    {"res",     0x80, 0x00, 0x00, 0x00, 0x00, 0x00, emit_bit},
    {"set",     0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, emit_bit},

    {"ldi",     0xed, 0xa0, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"ldir",    0xed, 0xb0, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"cpi",     0xed, 0xa1, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"cpir",    0xed, 0xb1, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"ini",     0xed, 0xa2, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"inir",    0xed, 0xb2, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"outi",    0xed, 0xa3, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"otir",    0xed, 0xb3, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"ldd",     0xed, 0xa8, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"lddr",    0xed, 0xb8, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"cpd",     0xed, 0xa9, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"cpdr",    0xed, 0xb9, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"ind",     0xed, 0xaa, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"indr",    0xed, 0xba, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"outd",    0xed, 0xab, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"otdr",    0xed, 0xbb, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"reti",    0xed, 0x4d, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"rrd",     0xed, 0x67, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"rld",     0xed, 0x6f, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"retn",    0xed, 0x45, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"neg",     0xed, 0x44, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
    {"im",      0x46, 0x56, 0x5e, 0xed, 0x00, 0x00, emit_im},
    {NULL,      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, NULL}
};