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

// Attempts to retrieve an 8-bit register from the argument, optionally including main pointers (HL, IX, IY).
// Tenta obter um registrador de 8 bits a partir do argumento, opcionalmente incluindo ponteiros principais (HL, IX, IY).
static reg_t *tryget_reg8bit(expr_t *arg, bool include_main_ptr)
{
    if (arg->token == TOK_REGISTER && (arg->reg->group & REG_8BIT))
        return arg->reg;
    if (include_main_ptr && arg->token == TOK_INDEX_OPEN && arg->right->token == TOK_REGISTER && (arg->right->reg->group & REG_MAIN_PTR))
        return arg->right->reg;
    if (include_main_ptr && arg->token == TOK_INDEX_OPEN && (arg->right->token == TOK_ADD || arg->right->token == TOK_SUB) && arg->right->left && arg->right->left->token == TOK_REGISTER && (arg->right->left->reg->group & REG_MAIN_PTR))
        return arg->right->left->reg;
    return NULL;
}

// Attempts to retrieve a register matching a specific group from the argument.
// Tenta obter um registrador que corresponda a um grupo específico a partir do argumento.
static reg_t *tryget_reg(expr_t *arg, int group)
{
    if (arg->token == TOK_REGISTER && (arg->reg->group & group))
        return arg->reg;
    if (arg->token == TOK_INDEX_OPEN && arg->right->token == TOK_REGISTER && (arg->right->reg->group & group))
        return arg->right->reg;
    if (arg->token == TOK_INDEX_OPEN && (arg->right->token == TOK_ADD || arg->right->token == TOK_SUB) && arg->right->left && arg->right->left->token == TOK_REGISTER && (arg->right->left->reg->group & group))
        return arg->right->left->reg;
    return NULL;
}

// Checks whether the expression is an indirect reference using an "other pointer" register (BC, DE).
// Verifica se a expressão é uma referência indireta usando um registrador "other pointer" (BC, DE).
static bool is_other_pointer(expr_t *e)
{
    if (e->token != TOK_INDEX_OPEN)
        return false;
    if (e->right->token != TOK_REGISTER)
        return false;
    return e->right->reg->group & REG_OTHER_PTR;
}

// Checks whether the expression is an indirect reference using a "main pointer" register (HL, IX, IY).
// Verifica se a expressão é uma referência indireta usando um registrador "main pointer" (HL, IX, IY).
static bool is_main_pointer(expr_t *e)
{
    if (e->token != TOK_INDEX_OPEN)
        return false;
    if (e->right->token == TOK_REGISTER && (e->right->reg->group & REG_MAIN_PTR))
        return true;
    if ((e->right->token == TOK_ADD || e->right->token == TOK_SUB) && e->right->right->token == TOK_REGISTER && (e->right->right->reg->group & REG_MAIN_PTR))
        return true;
    return false;
}

// Checks whether the expression is the A register (accumulator).
// Verifica se a expressão é o registrador A (acumulador).
static bool is_a_register(expr_t *e)
{
    return e->token == TOK_REGISTER && (e->reg->group & REG_8BIT) && e->reg->value == 7;
}

// Checks whether the expression is the HL, IX, or IY 16-bit register (value == 6 in the 16-bit group).
// Verifica se a expressão é o registrador de 16 bits HL, IX ou IY (value == 6 no grupo 16-bit).
static bool is_hl_ix_iy_register(expr_t *e)
{
    return e->token == TOK_REGISTER && (e->reg->group & REG_16BIT) && e->reg->value == 6;
}

// Checks whether the expression is any 8-bit register.
// Verifica se a expressão é qualquer registrador de 8 bits.
static bool is_8bit_register(expr_t *e)
{
    return e->token == TOK_REGISTER && (e->reg->group & REG_8BIT);
}

// Checks whether the expression is a 16-bit register that can also be used as SP (stack pointer).
// Verifica se a expressão é um registrador de 16 bits que também pode ser usado como SP (ponteiro de pilha).
static bool is_16bitsp_register(expr_t *e)
{
    return e->token == TOK_REGISTER && (e->reg->group & REG_16BIT_SP);
}

// Checks whether the expression is a register matching a given group.
// Verifica se a expressão é um registrador que corresponde a um determinado grupo.
static bool is_register(expr_t *e, int group)
{
    return e->token == TOK_REGISTER && (e->reg->group & group);
}

// Checks whether the expression tree evaluates to a value-only expression (no registers or indirect references).
// Verifica se a árvore de expressão resulta em uma expressão somente de valor (sem registradores ou referências indiretas).
static bool is_value_only(expr_t *e)
{
    switch (e->token)
    {
    case TOK_VALUE:
        return true;
    case TOK_SYMBOL:
        return true;
    case TOK_SUB_LABEL:
        return true;
    case TOK_CURRENT_POS:
        return true;
    case TOK_MNEMONIC:
        return false;
    case TOK_REGISTER:
        return false;
    case TOK_INDEX_OPEN:
        return false;
    default:
        // Recursively check children for value-only expressions.
        // Verifica recursivamente os filhos para expressões somente de valor.
        if (e->left && !is_value_only(e->left))
            return false;
        if (e->right && !is_value_only(e->right))
            return false;
        return true;
    }
}

// Checks whether the expression is an indirect memory reference with a value-only address (e.g., (label)).
// Verifica se a expressão é uma referência indireta à memória com um endereço somente de valor (ex.: (label)).
static bool is_address_only(expr_t *e)
{
    if (e->token != TOK_INDEX_OPEN)
        return false;
    return is_value_only(e->right);
}

// Emits a single-byte opcode with no arguments.
// Emite um opcode de um único byte sem argumentos.
static void emit_simple(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    if (argc != 0)
        error_expr(mnemonic, "invalid argument count.");
    out(REC_DATA, 0, 0, &opcode->op1, 1);
}

// Emits a two-byte opcode with no arguments.
// Emite um opcode de dois bytes sem argumentos.
static void emit_simple_2bytes(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    if (argc != 0)
        error_expr(mnemonic, "invalid argument count.");
    out(REC_DATA, 0, 0, &opcode->op1, 1);
    out(REC_DATA, 0, 0, &opcode->op2, 1);
}

// Emits opcodes for logical/arithmetic operations (ADD, ADC, SUB, SBC, AND, XOR, OR, CP, RLC, RRC, RL, RR, SLA, SRA, SLL, SRL).
// Emite opcodes para operações lógicas/aritméticas (ADD, ADC, SUB, SBC, AND, XOR, OR, CP, RLC, RRC, RL, RR, SLA, SRA, SLL, SRL).
static void emit_logic(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op;
    reg_t *reg1 = NULL;
    reg_t *reg2 = NULL;
    expr_t *arg = NULL;
    bool is_8bit_op = false;
    bool is_16bit_op = false;
    bool is_8bit_value = false;

    // Parse arguments: handles 2-operand and 1-operand forms.
    // Analisa argumentos: lida com formas de 2 operandos e 1 operando.
    if (argc == 2)
    {
        // Both operands are 8-bit registers or combinations with index registers.
        // Ambos os operandos são registradores de 8 bits ou combinações com registradores de índice.
        if ((reg1 = tryget_reg8bit(argv[0], true)) != NULL)
        {
            if ((reg2 = tryget_reg8bit(argv[1], true)) != NULL)
            {
                arg = argv[1];
                is_8bit_op = true;
                // Swap operands if reg1 is not A and reg2 is A (opcode symmetry).
                // Troca operandos se reg1 não é A e reg2 é A (simetria do opcode).
                if (!opcode->op5 && reg1->value != 7 && reg2->value == 7)
                {
                    reg_t *tmp = reg1;
                    reg1 = reg2;
                    reg2 = tmp;
                    arg = argv[0];
                }
            }
            else
            {
                is_8bit_value = true;
                reg2 = NULL;
                arg = argv[1];
            }
        }
        // First operand is a main pointer (HL, IX, IY indirect reference).
        // Primeiro operando é um ponteiro principal (referência indireta HL, IX, IY).
         else if ((reg1 = tryget_reg(argv[0], REG_MAIN_PTR)) != NULL)
         {
             // Emit IX prefix if needed.
             // Emite prefixo IX se necessário.
             if ((reg1->group & (REG_IX_PTR)))
             {
                 op = 0xdd;
                 out(REC_DATA, 0, 0, &op, 1);
             }
             // Emit IY prefix if needed.
             // Emite prefixo IY se necessário.
             if ((reg1->group & (REG_IY_PTR)))
             {
                 op = 0xfd;
                 out(REC_DATA, 0, 0, &op, 1);
             }
             reg2 = tryget_reg(argv[1], REG_16BIT_SP);
             // Validate register combinations for 16-bit add operations.
             // Valida combinações de registradores para operações ADD de 16 bits.
             if (reg2)
             {
                 int r1_is_hl = (reg1->group & (REG_IX_PTR | REG_IY_PTR)) == 0;
                 int r2_is_index = (reg2->group & (REG_IX_PTR | REG_IY_PTR)) != 0;
                 if (r1_is_hl && r2_is_index)
                     error_expr(argv[1], "invalid register combination: add hl, ix/iy not supported");
                 int r1_is_index = (reg1->group & (REG_IX_PTR | REG_IY_PTR)) != 0;
                 int r2_is_hl = (reg2->group & (REG_IX_PTR | REG_IY_PTR)) == 0 && (reg2->group & REG_16BIT);
                 if (r1_is_index && r2_is_hl)
                     error_expr(argv[1], "invalid register combination: add ix/iy, hl not supported");
             }
             arg = argv[1];
             is_16bit_op = true;
         }
    }
    else if (argc == 1)
    {
        // Single 8-bit register operand (implicit A).
        // Operando único de registrador de 8 bits (A implícito).
        if ((reg2 = tryget_reg8bit(argv[0], true)) != NULL)
        {
            reg1 = &_regs[7];
            arg = argv[0];
            is_8bit_op = true;
        }
        // Single 16-bit register operand (implicit HL).
        // Operando único de registrador de 16 bits (HL implícito).
        else if ((reg2 = tryget_reg(argv[0], REG_16BIT_SP)) != NULL)
        {
            reg2 = &_regs[6];
            arg = argv[1];
            is_16bit_op = true;
        }
        // Single value operand (implicit A).
        // Operando único de valor (A implícito).
        else
        {
            arg = argv[0];
            is_8bit_value = true;
        }
    }
    else
        error_expr(mnemonic, "invalid argument count.");

    // Emit 8-bit register-to-register operation.
    // Emite operação de 8 bits entre registradores.
    if (is_8bit_op && reg1 && reg2 && (opcode->op1 || opcode->op6 == 0xff))
    {
        // Emit IX prefix if reg1 is IX-based.
        // Emite prefixo IX se reg1 for baseado em IX.
        if (reg1->group & REG_IX_PTR)
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        else if (reg1->group & REG_IY_PTR)
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        // Emit IX prefix if reg2 is IX-based.
        // Emite prefixo IX se reg2 for baseado em IX.
        if (reg2->group & REG_IX_PTR)
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        else if (reg2->group & REG_IY_PTR)
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        // Handle CB-prefixed operations (shift/rotate with index displacement).
        // Lida com operações prefixadas com CB (deslocamento/rotação com deslocamento de índice).
        if (opcode->op5)
        {
            op = opcode->op5;
            out(REC_DATA, 0, 0, &op, 1);
            // Emit displacement for IX/IY indirect operands.
            // Emite deslocamento para operandos indiretos IX/IY.
            if (reg1->group & (REG_IX_PTR | REG_IY_PTR))
            {
                expr_t *disp = filter_registers(argv[0]);
                if (generate(disp->right, 2, false))
                {
                    out(REC_EXPR_PUSH_OFFSET, 2, 0, 0, 0);
                    out(REC_EXPR_SUB, 0, 0, 0, 0);
                }
                out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
            }
            else if (reg2->group & (REG_IX_PTR | REG_IY_PTR))
            {
                arg = filter_registers(arg);
                if (generate(arg->right, 2, false))
                {
                    out(REC_EXPR_PUSH_OFFSET, 2, 0, 0, 0);
                    out(REC_EXPR_SUB, 0, 0, 0, 0);
                }
                out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
            }
            op = opcode->op1 | reg2->value;
            out(REC_DATA, 0, 0, &op, 1);
        }
        else
        {
            op = opcode->op1 | reg2->value;
            out(REC_DATA, 0, 0, &op, 1);
            // Emit displacement for IX/IY indirect operands.
            // Emite deslocamento para operandos indiretos IX/IY.
            if (reg1->group & (REG_IX_PTR | REG_IY_PTR))
            {
                expr_t *disp = filter_registers(argv[0]);
                if (generate(disp->right, 2, false))
                {
                    out(REC_EXPR_PUSH_OFFSET, 2, 0, 0, 0);
                    out(REC_EXPR_SUB, 0, 0, 0, 0);
                }
                out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
            }
            else if (reg2->group & (REG_IX_PTR | REG_IY_PTR))
            {
                arg = filter_registers(arg);
                if (generate(arg->right, 2, false))
                {
                    out(REC_EXPR_PUSH_OFFSET, 2, 0, 0, 0);
                    out(REC_EXPR_SUB, 0, 0, 0, 0);
                }
                out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
            }
        }
    }
    // Emit 16-bit register operation.
    // Emite operação de registrador de 16 bits.
    else if (is_16bit_op && reg1 && reg2 && opcode->op2)
    {
        // Emit prefix opcode if present (e.g., ED for ADC/SBC).
        // Emite opcode de prefixo se presente (ex.: ED para ADC/SBC).
        if (opcode->op4)
        {
            op = opcode->op4;
            out(REC_DATA, 0, 0, &op, 1);
        }
        // Emit IX prefix if needed.
        // Emite prefixo IX se necessário.
        if (reg2->group & REG_IX_PTR)
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        else if (reg2->group & REG_IY_PTR)
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = opcode->op2 | reg2->value_aux << 4;
        out(REC_DATA, 0, 0, &op, 1);
    }
    // Emit 8-bit operation with immediate value and register A as destination.
    // Emite operação de 8 bits com valor imediato e registrador A como destino.
    else if (is_8bit_value && reg1 && reg1->value == 7 && opcode->op3)
    {
        op = opcode->op3;
        out(REC_DATA, 0, 0, &op, 1);
        generate(arg, 0, false);
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
    }
    // Emit 8-bit operation with immediate value and explicit register destination.
    // Emite operação de 8 bits com valor imediato e destino explícito de registrador.
    else if (is_8bit_value && !reg1 && opcode->op3)
    {
        op = opcode->op3;
        out(REC_DATA, 0, 0, &op, 1);
        generate(arg, 0, false);
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
    }
    else
        error_expr(argv[0], "invalid arguments");
}

// Emits a one-byte opcode followed by a relative 8-bit offset (for JR, DJNZ).
// Emite um opcode de um byte seguido por um deslocamento relativo de 8 bits (para JR, DJNZ).
static void emit_offset8bit(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    if (argc != 1)
        error_expr(mnemonic, "invalid argument count.");
    out(REC_DATA, 0, 0, &opcode->op1, 1);
    if (generate(argv[0], 0, false))
    {
        out(REC_EXPR_PUSH_OFFSET, 1, 0, 0, 0);
        out(REC_EXPR_SUB, 0, 0, 0, 0);
    }
    out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
}

// Emits a conditional jump or relative branch with an 8-bit offset (JR cc, offset).
// Emite um desvio condicional ou ramificação relativa com deslocamento de 8 bits (JR cc, deslocamento).
static void emit_cmp__offset8bit(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op = opcode->op1;

    // Two arguments: condition flag and target offset.
    // Dois argumentos: flag de condição e deslocamento de destino.
    if (argc == 2)
    {
        reg_t *cmp = tryget_reg(argv[0], REG_CMP);
        if (!cmp)
            error_expr(argv[0], "flag expected.");
        op |= cmp->value_aux << 3;
        out(REC_DATA, 0, 0, &op, 1);
        if (generate(argv[1], 0, false))
        {
            out(REC_EXPR_PUSH_OFFSET, 1, 0, 0, 0);
            out(REC_EXPR_SUB, 0, 0, 0, 0);
        }
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
    }
    // Single argument: unconditional branch.
    // Argumento único: desvio incondicional.
    else if (argc == 1 && opcode->op2)
    {
        op = opcode->op2;
        out(REC_DATA, 0, 0, &op, 1);
        if (generate(argv[0], 0, false))
        {
            out(REC_EXPR_PUSH_OFFSET, 1, 0, 0, 0);
            out(REC_EXPR_SUB, 0, 0, 0, 0);
        }
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
    }
    else
        error_expr(mnemonic, "invalid argument count.");
}

// Emits a conditional or unconditional jump/call with a 16-bit offset (JP, CALL).
// Emite um salto/chamada condicional ou incondicional com deslocamento de 16 bits (JP, CALL).
static void emit_cmp__offset16bit(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op = opcode->op1;

    // Two arguments: condition flag and target address.
    // Dois argumentos: flag de condição e endereço de destino.
    if (argc == 2)
    {
        reg_t *cmp = tryget_reg(argv[0], REG_CMP);
        if (!cmp)
            error_expr(argv[0], "flag expected.");
        op |= cmp->value_aux << 3;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[1], -1, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Single argument: jump via main pointer register (e.g., JP (HL), JP (IX), JP (IY)).
    // Argumento único: salto via registrador ponteiro principal (ex.: JP (HL), JP (IX), JP (IY)).
    else if (argc == 1 && argv[0]->token == TOK_INDEX_OPEN && argv[0]->right->token == TOK_REGISTER && (argv[0]->right->reg->group & REG_MAIN_PTR))
    {
        // Emit IX prefix if needed.
        // Emite prefixo IX se necessário.
        if (argv[0]->right->reg->group & REG_IX_PTR)
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        else if (argv[0]->right->reg->group & REG_IY_PTR)
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = 0xe9;
        out(REC_DATA, 0, 0, &op, 1);
    }
    // Single argument: unconditional jump/call to a 16-bit address.
    // Argumento único: salto/chamada incondicional para um endereço de 16 bits.
    else if (argc == 1 && opcode->op2)
    {
        op = opcode->op2;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[0], -1, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    else
        error_expr(mnemonic, "invalid argument count.");
}

// Emits a conditional or unconditional return instruction (RET).
// Emite uma instrução de retorno condicional ou incondicional (RET).
static void emit_cmp(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op = opcode->op1;

    // One argument: conditional return with flag.
    // Um argumento: retorno condicional com flag.
    if (argc == 1)
    {
        reg_t *cmp = tryget_reg(argv[0], REG_CMP);
        if (!cmp)
            error_expr(argv[0], "flag expected.");
        op |= cmp->value_aux << 3;
        out(REC_DATA, 0, 0, &op, 1);
    }
    // No arguments: unconditional return.
    // Nenhum argumento: retorno incondicional.
    else if (argc == 0 && opcode->op2)
    {
        op = opcode->op2;
        out(REC_DATA, 0, 0, &op, 1);
    }
    else
        error_expr(mnemonic, "invalid argument count.");
}

// Emits instructions that accept 16-bit AF registers, 8-bit registers, or index registers (POP, PUSH).
// Emite instruções que aceitam registradores AF de 16 bits, registradores de 8 bits ou registradores de índice (POP, PUSH).
static void emit_reg16bitaf_or_reg8bit_or_index(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op;
    if (argc != 1)
        error_expr(mnemonic, "invalid argument count.");

    // Try 16-bit AF register (BC, DE, HL, IX, IY, SP, AF).
    // Tenta registrador AF de 16 bits (BC, DE, HL, IX, IY, SP, AF).
    reg_t *reg = tryget_reg(argv[0], REG_16BIT_AF);
    if (reg && argv[0]->token != TOK_INDEX_OPEN && opcode->op1)
    {
        // Emit IX prefix if the register is IX.
        // Emite prefixo IX se o registrador for IX.
        if (reg->group & REG_IX_PTR)
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        else if (reg->group & REG_IY_PTR)
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = opcode->op1 | reg->value_aux << 4;
        out(REC_DATA, 0, 0, &op, 1);
        return;
    }

    // Try 8-bit register or index register indirect reference.
    // Tenta registrador de 8 bits ou referência indireta de registrador de índice.
    reg = tryget_reg8bit(argv[0], true);
    if (reg && opcode->op2)
    {
        // Emit IX prefix if needed.
        // Emite prefixo IX se necessário.
        if (reg->group & REG_IX_PTR)
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        else if (reg->group & REG_IY_PTR)
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = opcode->op2 | reg->value << 3;
        out(REC_DATA, 0, 0, &op, 1);
        // Emit displacement for IX/IY indirect operands.
        // Emite deslocamento para operandos indiretos IX/IY.
        if ((reg->group & REG_IX_PTR) || (reg->group & REG_IY_PTR))
        {
            argv[0] = filter_registers(argv[0]);
            if (generate(argv[0]->right, 2, false))
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

// Emits instructions that accept 16-bit SP registers, 8-bit registers, or index registers (INC, DEC).
// Emite instruções que aceitam registradores SP de 16 bits, registradores de 8 bits ou registradores de índice (INC, DEC).
static void emit_reg16bitsp_or_reg8bit_or_index(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op;
    if (argc != 1)
        error_expr(mnemonic, "invalid argument count.");

    // Try 16-bit SP register (BC, DE, HL, IX, IY, SP).
    // Tenta registrador SP de 16 bits (BC, DE, HL, IX, IY, SP).
    reg_t *reg = tryget_reg(argv[0], REG_16BIT_SP);
    if (reg && argv[0]->token != TOK_INDEX_OPEN && opcode->op1)
    {
        // Emit IX prefix if the register is IX.
        // Emite prefixo IX se o registrador for IX.
        if (reg->group & REG_IX_PTR)
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        else if (reg->group & REG_IY_PTR)
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = opcode->op1 | reg->value_aux << 4;
        out(REC_DATA, 0, 0, &op, 1);
        return;
    }

    // Try 8-bit register or index register indirect reference.
    // Tenta registrador de 8 bits ou referência indireta de registrador de índice.
    reg = tryget_reg8bit(argv[0], true);
    if (reg && opcode->op2)
    {
        // Emit IX prefix if needed.
        // Emite prefixo IX se necessário.
        if (reg->group & REG_IX_PTR)
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        else if (reg->group & REG_IY_PTR)
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = opcode->op2 | reg->value << 3;
        out(REC_DATA, 0, 0, &op, 1);
        // Emit displacement for IX/IY indirect operands.
        // Emite deslocamento para operandos indiretos IX/IY.
        if ((reg->group & REG_IX_PTR) || (reg->group & REG_IY_PTR))
        {
            argv[0] = filter_registers(argv[0]);
            if (generate(argv[0]->right, 2, false))
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

// Emits the RST (restart) instruction with a fixed value (0x00, 0x08, ..., 0x38).
// Emite a instrução RST (restart) com um valor fixo (0x00, 0x08, ..., 0x38).
static void emit_rst(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op;
    if (argc != 1)
        error_expr(mnemonic, "invalid argument count.");
    if (argv[0]->token != TOK_VALUE)
        error_expr(argv[0], "value expected.");
    if ((argv[0]->value & 0x38) != argv[0]->value)
        error_expr(argv[0], "invalid rst value.");
    op = opcode->op1 | argv[0]->value;
    out(REC_DATA, 0, 0, &op, 1);
}

// Emits the EX (exchange) instruction, supporting various register exchange combinations.
// Emite a instrução EX (troca), suportando várias combinações de troca de registradores.
static void emit_ex(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op;
    if (argc != 2)
        error_expr(mnemonic, "invalid argument count.");

    // Extract register pointers from both arguments (direct and indirect).
    // Extrai ponteiros de registradores de ambos os argumentos (direto e indireto).
    reg_t *reg1 = argv[0]->token == TOK_REGISTER ? argv[0]->reg : NULL;
    reg_t *reg1idx = argv[0]->token == TOK_INDEX_OPEN && argv[0]->right->token == TOK_REGISTER ? argv[0]->right->reg : NULL;
    reg_t *reg2 = argv[1]->token == TOK_REGISTER ? argv[1]->reg : NULL;
    reg_t *reg2idx = argv[1]->token == TOK_INDEX_OPEN && argv[1]->right->token == TOK_REGISTER ? argv[1]->right->reg : NULL;

    // EX AF, AF' (alternate register set).
    // EX AF, AF' (conjunto alternativo de registradores).
    if (reg1 && reg2 && (reg1->group & REG_16BIT_AF) && reg1->value == 3 && (reg2->group & REG_16BIT_ALT) && reg2->value == 0)
    {
        out(REC_DATA, 0, 0, &opcode->op1, 1);
    }
    // EX AF, AF (self-exchange, also opcode 0x08).
    // EX AF, AF (auto-troca, também opcode 0x08).
    else if (reg1 && reg2 && (reg1->group & REG_16BIT_AF) && reg1->value == 3 && (reg2->group & REG_16BIT_AF) && reg2->value == 3)
    {
        out(REC_DATA, 0, 0, &opcode->op1, 1);
    }
    // EX (SP), IX.
    // EX (SP), IX.
    else if (reg1idx && reg2 && (reg1idx->group & REG_16BIT_SP) && reg1idx->value == 3 && (reg2->group & REG_IX_PTR))
    {
        op = 0xdd;
        out(REC_DATA, 0, 0, &op, 1);
        out(REC_DATA, 0, 0, &opcode->op2, 1);
    }
    // EX (SP), IY.
    // EX (SP), IY.
    else if (reg1idx && reg2 && (reg1idx->group & REG_16BIT_SP) && reg1idx->value == 3 && (reg2->group & REG_IY_PTR))
    {
        op = 0xfd;
        out(REC_DATA, 0, 0, &op, 1);
        out(REC_DATA, 0, 0, &opcode->op2, 1);
    }
    // EX (SP), HL.
    // EX (SP), HL.
    else if (reg1idx && reg2 && (reg1idx->group & REG_16BIT_SP) && reg1idx->value == 3 && (reg2->group & REG_16BIT_AF) && (reg2->group & (REG_IX_PTR | REG_IY_PTR)) == 0 && reg2->value == 6)
    {
        out(REC_DATA, 0, 0, &opcode->op2, 1);
    }
    // Reverse: EX IX, (SP).
    // Reverso: EX IX, (SP).
    else if (reg2idx && reg1 && (reg2idx->group & REG_16BIT_SP) && reg2idx->value == 3 && (reg1->group & REG_IX_PTR))
    {
        op = 0xdd;
        out(REC_DATA, 0, 0, &op, 1);
        out(REC_DATA, 0, 0, &opcode->op2, 1);
    }
    // Reverse: EX IY, (SP).
    // Reverso: EX IY, (SP).
    else if (reg2idx && reg1 && (reg2idx->group & REG_16BIT_SP) && reg2idx->value == 3 && (reg1->group & REG_IY_PTR))
    {
        op = 0xfd;
        out(REC_DATA, 0, 0, &op, 1);
        out(REC_DATA, 0, 0, &opcode->op2, 1);
    }
    // Reverse: EX HL, (SP).
    // Reverso: EX HL, (SP).
    else if (reg2idx && reg1 && (reg2idx->group & REG_16BIT_SP) && reg2idx->value == 3 && (reg1->group & REG_16BIT_AF) && (reg1->group & (REG_IX_PTR | REG_IY_PTR)) == 0 && reg1->value == 6)
    {
        out(REC_DATA, 0, 0, &opcode->op2, 1);
    }
    // EX DE, HL.
    // EX DE, HL.
    else if (reg1 && reg2 && (reg1->group & REG_16BIT_AF) && reg1->value == 1 && (reg2->group & REG_16BIT_AF) && (reg2->group & (REG_IX_PTR | REG_IY_PTR)) == 0 && reg2->value == 6)
    {
        out(REC_DATA, 0, 0, &opcode->op3, 1);
    }
    // Reverse: EX HL, DE.
    // Reverso: EX HL, DE.
    else if (reg1 && reg2 && (reg2->group & REG_16BIT_AF) && reg2->value == 1 && (reg1->group & REG_16BIT_AF) && (reg1->group & (REG_IX_PTR | REG_IY_PTR)) == 0 && reg1->value == 6)
    {
        out(REC_DATA, 0, 0, &opcode->op3, 1);
    }
    else
        error_expr(argv[0], "invalid arguments.");
}

// Emits the LD (load) instruction, handling all Z80 load variants between registers, memory, and values.
// Emite a instrução LD (carregamento), lidando com todas as variantes de carga Z80 entre registradores, memória e valores.
static void emit_ld(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op;
    reg_t *reg1;
    reg_t *reg2;
    if (argc != 2)
        error_expr(mnemonic, "invalid argument count.");

    // LD r8, r8: 8-bit register to 8-bit register (including IX/IY indirect).
    // LD r8, r8: registrador de 8 bits para registrador de 8 bits (incluindo indireto IX/IY).
    if ((reg1 = tryget_reg8bit(argv[0], true)) && (reg2 = tryget_reg8bit(argv[1], true)))
    {
        // Both operands cannot be main pointer indirect references.
        // Ambos os operandos não podem ser referências indiretas de ponteiro principal.
        if ((reg1->group & REG_MAIN_PTR) && (reg2->group & REG_MAIN_PTR))
        {
            error_expr(argv[0], "invalid argument combination.");
        }
        // Emit IX prefix if either operand uses IX.
        // Emite prefixo IX se algum operando usar IX.
        if ((reg1->group & (REG_IX_PTR)) || (reg2->group & (REG_IX_PTR)))
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        // Emit IY prefix if either operand uses IY.
        // Emite prefixo IY se algum operando usar IY.
        if ((reg1->group & (REG_IY_PTR)) || (reg2->group & (REG_IY_PTR)))
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = 0x40 | (reg1->value << 3) | reg2->value;
        out(REC_DATA, 0, 0, &op, 1);
        // Emit displacement for IX/IY destination.
        // Emite deslocamento para destino IX/IY.
        if ((reg1->group & (REG_IX_PTR | REG_IY_PTR)))
        {
            argv[0] = filter_registers(argv[0]);
            if (generate(argv[0]->right, 2, false))
            {
                out(REC_EXPR_PUSH_OFFSET, 2, 0, 0, 0);
                out(REC_EXPR_SUB, 0, 0, 0, 0);
            }
            out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
        }
        // Emit displacement for IX/IY source.
        // Emite deslocamento para origem IX/IY.
        if ((reg2->group & (REG_IX_PTR | REG_IY_PTR)))
        {
            argv[1] = filter_registers(argv[1]);
            if (generate(argv[1]->right, 2, false))
            {
                out(REC_EXPR_PUSH_OFFSET, 2, 0, 0, 0);
                out(REC_EXPR_SUB, 0, 0, 0, 0);
            }
            out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
        }
    }
    // LD r8, imm8: 8-bit register loaded with immediate value.
    // LD r8, imm8: registrador de 8 bits carregado com valor imediato.
    else if ((reg1 = tryget_reg8bit(argv[0], true)) && is_value_only(argv[1]))
    {
        // Emit IX prefix if needed.
        // Emite prefixo IX se necessário.
        if ((reg1->group & (REG_IX_PTR)))
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        if ((reg1->group & (REG_IY_PTR)))
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = 0x06 | (reg1->value << 3);
        out(REC_DATA, 0, 0, &op, 1);
        // Emit displacement for IX/IY indirect destination.
        // Emite deslocamento para destino indireto IX/IY.
        if ((reg1->group & (REG_IX_PTR | REG_IY_PTR)))
        {
            argv[0] = filter_registers(argv[0]);
            if (generate(argv[0]->right, 2, false))
            {
                out(REC_EXPR_PUSH_OFFSET, 2, 0, 0, 0);
                out(REC_EXPR_SUB, 0, 0, 0, 0);
            }
            out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
        }
        generate(argv[1], 1, false);
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
    }
    // LD (BC/DE), A: store A to memory via BC or DE.
    // LD (BC/DE), A: armazena A na memória via BC ou DE.
    else if (is_other_pointer(argv[0]) && is_a_register(argv[1]) && (reg1 = tryget_reg(argv[0], REG_OTHER_PTR)))
    {
        op = 0x02 | reg1->value << 4;
        out(REC_DATA, 0, 0, &op, 1);
    }
    // LD A, (BC/DE): load A from memory via BC or DE.
    // LD A, (BC/DE): carrega A da memória via BC ou DE.
    else if (is_other_pointer(argv[1]) && is_a_register(argv[0]) && (reg2 = tryget_reg(argv[1], REG_OTHER_PTR)))
    {
        op = 0x0a | reg2->value << 4;
        out(REC_DATA, 0, 0, &op, 1);
    }
    // LD r16, imm16: 16-bit register loaded with immediate 16-bit value.
    // LD r16, imm16: registrador de 16 bits carregado com valor imediato de 16 bits.
    else if (is_16bitsp_register(argv[0]) && is_value_only(argv[1]) && (reg1 = tryget_reg(argv[0], REG_16BIT_SP)))
    {
        // Emit IX prefix if needed.
        // Emite prefixo IX se necessário.
        if ((reg1->group & (REG_IX_PTR)))
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        if ((reg1->group & (REG_IY_PTR)))
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = 0x01 | reg1->value_aux << 4;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[1], 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // LD (imm16), HL/IX/IY: store HL/IX/IY to memory at 16-bit address.
    // LD (imm16), HL/IX/IY: armazena HL/IX/IY na memória em endereço de 16 bits.
    else if (is_address_only(argv[0]) && is_hl_ix_iy_register(argv[1]) && (reg1 = tryget_reg(argv[1], REG_MAIN_PTR)))
    {
        // Emit IX prefix if needed.
        // Emite prefixo IX se necessário.
        if ((reg1->group & (REG_IX_PTR)))
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        if ((reg1->group & (REG_IY_PTR)))
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = 0x22;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[0]->right, 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // LD (imm16), A: store accumulator to memory at 16-bit address.
    // LD (imm16), A: armazena acumulador na memória em endereço de 16 bits.
    else if (is_address_only(argv[0]) && is_a_register(argv[1]) && (reg1 = tryget_reg(argv[1], REG_8BIT)))
    {
        op = 0x32;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[0]->right, 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // LD HL/IX/IY, (imm16): load HL/IX/IY from memory at 16-bit address.
    // LD HL/IX/IY, (imm16): carrega HL/IX/IY da memória em endereço de 16 bits.
    else if (is_address_only(argv[1]) && is_hl_ix_iy_register(argv[0]) && (reg1 = tryget_reg(argv[0], REG_MAIN_PTR)))
    {
        // Emit IX prefix if needed.
        // Emite prefixo IX se necessário.
        if ((reg1->group & (REG_IX_PTR)))
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        if ((reg1->group & (REG_IY_PTR)))
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = 0x2a;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[1]->right, 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // LD A, (imm16): load accumulator from memory at 16-bit address.
    // LD A, (imm16): carrega acumulador da memória em endereço de 16 bits.
    else if (is_address_only(argv[1]) && is_a_register(argv[0]) && (reg1 = tryget_reg(argv[0], REG_8BIT)))
    {
        op = 0x3a;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[1]->right, 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // LD SP, HL/IX/IY: set stack pointer to HL/IX/IY.
    // LD SP, HL/IX/IY: define ponteiro de pilha para HL/IX/IY.
    else if (is_register(argv[0], REG_SP) && is_register(argv[1], REG_MAIN_PTR) && (reg1 = tryget_reg(argv[1], REG_MAIN_PTR)))
    {
        // Emit IX prefix if needed.
        // Emite prefixo IX se necessário.
        if ((reg1->group & (REG_IX_PTR)))
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        if ((reg1->group & (REG_IY_PTR)))
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        op = 0xf9;
        out(REC_DATA, 0, 0, &op, 1);
    }
    // LD (imm16), r16: store 16-bit register to memory at 16-bit address (ED-prefixed).
    // LD (imm16), r16: armazena registrador de 16 bits na memória em endereço de 16 bits (prefixo ED).
    else if (is_address_only(argv[0]) && is_16bitsp_register(argv[1]) && (reg1 = tryget_reg(argv[1], REG_16BIT_SP)))
    {
        op = 0xed;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0x43 | (reg1->value_aux << 4);
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[0]->right, 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // LD r16, (imm16): load 16-bit register from memory at 16-bit address (ED-prefixed).
    // LD r16, (imm16): carrega registrador de 16 bits da memória em endereço de 16 bits (prefixo ED).
    else if (is_address_only(argv[1]) && is_16bitsp_register(argv[0]) && (reg1 = tryget_reg(argv[0], REG_16BIT_SP)))
    {
        op = 0xed;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0x4b | (reg1->value_aux << 4);
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[1]->right, 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // LD A, I: load interrupt vector register I into accumulator.
    // LD A, I: carrega registrador de vetor de interrupção I no acumulador.
    else if (is_a_register(argv[0]) && is_register(argv[1], REG_I))
    {
        op = 0xed;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0x57;
        out(REC_DATA, 0, 0, &op, 1);
    }
    // LD I, A: store accumulator into interrupt vector register I.
    // LD I, A: armazena acumulador no registrador de vetor de interrupção I.
    else if (is_a_register(argv[1]) && is_register(argv[0], REG_I))
    {
        op = 0xed;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0x47;
        out(REC_DATA, 0, 0, &op, 1);
    }
    // LD A, R: load memory refresh register R into accumulator.
    // LD A, R: carrega registrador de refresh de memória R no acumulador.
    else if (is_a_register(argv[0]) && is_register(argv[1], REG_R))
    {
        op = 0xed;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0x5f;
        out(REC_DATA, 0, 0, &op, 1);
    }
    // LD R, A: store accumulator into memory refresh register R.
    // LD R, A: armazena acumulador no registrador de refresh de memória R.
    else if (is_a_register(argv[1]) && is_register(argv[0], REG_R))
    {
        op = 0xed;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0x4f;
        out(REC_DATA, 0, 0, &op, 1);
    }
    else
        error_expr(argv[0], "invalid destination/source combination.");
}

// Emits BIT, RES, or SET instructions for bit operations on registers or memory.
// Emite instruções BIT, RES ou SET para operações de bit em registradores ou memória.
static void emit_bit(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op;
    reg_t *reg;
    if (argc != 2)
        error_expr(mnemonic, "invalid argument count.");

    // First argument is the bit number, second is the target register or memory.
    // Primeiro argumento é o número do bit, segundo é o registrador ou memória alvo.
    if (is_value_only(argv[0]) && (reg = tryget_reg8bit(argv[1], true)))
    {
        if (argv[0]->token != TOK_VALUE)
            error_expr(argv[0], "constant expression expected.");

        // Emit IX prefix if target uses IX.
        // Emite prefixo IX se o alvo usar IX.
        if (reg->group & REG_IX_PTR)
        {
            op = 0xdd;
            out(REC_DATA, 0, 0, &op, 1);
        }
        else if (reg->group & REG_IY_PTR)
        {
            op = 0xfd;
            out(REC_DATA, 0, 0, &op, 1);
        }

        // Emit CB prefix for bit operations.
        // Emite prefixo CB para operações de bit.
        op = 0xcb;
        out(REC_DATA, 0, 0, &op, 1);

        // Emit displacement for IX/IY indirect operands.
        // Emite deslocamento para operandos indiretos IX/IY.
        if ((reg->group & REG_IX_PTR) || (reg->group & REG_IY_PTR))
        {
            argv[1] = filter_registers(argv[1]);
            if (generate(argv[1]->right, 2, false))
            {
                out(REC_EXPR_PUSH_OFFSET, 2, 0, 0, 0);
                out(REC_EXPR_SUB, 0, 0, 0, 0);
            }
            out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
        }

        op = opcode->op1 | argv[0]->value << 3 | reg->value;
        out(REC_DATA, 0, 0, &op, 1);
    }
    else
        error_expr(argv[0], "invalid bit/register combination.");
}

// Emits the IM (interrupt mode) instruction (IM 0, IM 1, IM 2).
// Emite a instrução IM (modo de interrupção) (IM 0, IM 1, IM 2).
static void emit_im(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op;
    uint8_t ops[3];
    ops[0] = opcode->op1;
    ops[1] = opcode->op2;
    ops[2] = opcode->op3;
    reg_t *reg;
    if (argc != 1)
        error_expr(mnemonic, "invalid argument count.");

    // Validate argument is a constant value (0, 1, or 2) and emit the corresponding opcode.
    // Valida que o argumento é um valor constante (0, 1 ou 2) e emite o opcode correspondente.
    if (is_value_only(argv[0]) && argv[0]->token != TOK_VALUE && argv[0]->value < 3)
    {
        op = opcode->op4;
        out(REC_DATA, 0, 0, &op, 1);
        op = ops[argv[0]->value];
        out(REC_DATA, 0, 0, &op, 1);
    }
    else
        error_expr(argv[0], "invalid constant expression.");
}

// Emits the OUT instruction (output to I/O port).
// Emite a instrução OUT (saída para porta de E/S).
static void emit_out(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op;
    if (argc != 2)
        error_expr(mnemonic, "invalid argument count.");

    // OUT (imm8), A: output A to an 8-bit port address.
    // OUT (imm8), A: envia A para um endereço de porta de 8 bits.
    if (is_address_only(argv[0]) && is_a_register(argv[1]))
    {
        op = opcode->op1;
        out(REC_DATA, 0, 0, &op, 1);
        generate(argv[0]->right, 0, false);
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
    }
    // OUT (C), r8: output 8-bit register to port (C).
    // OUT (C), r8: envia registrador de 8 bits para a porta (C).
    else if (is_8bit_register(argv[1]) && tryget_reg(argv[0], REG_CMP))
    {
        reg_t *c_port = tryget_reg(argv[0], REG_CMP);
        if (c_port->value_aux != 3)
            error_expr(argv[0], "register C expected.");
        op = 0xed;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0x41 | (argv[1]->reg->value << 3);
        out(REC_DATA, 0, 0, &op, 1);
    }
    // OUT (C), 0: output zero to port (C).
    // OUT (C), 0: envia zero para a porta (C).
    else if (tryget_reg(argv[0], REG_CMP) && is_value_only(argv[1]))
    {
        reg_t *c_port = tryget_reg(argv[0], REG_CMP);
        if (c_port->value_aux != 3)
            error_expr(argv[0], "register C expected.");
        op = 0xed;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0x71;
        out(REC_DATA, 0, 0, &op, 1);
    }
    else
        error_expr(argv[0], "invalid arguments.");
}

// Emits the IN instruction (input from I/O port).
// Emite a instrução IN (entrada de porta de E/S).
static void emit_in(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op;
    if (argc != 2)
        error_expr(mnemonic, "invalid argument count.");

    // IN A, (imm8): input from an 8-bit port address into A.
    // IN A, (imm8): entrada de um endereço de porta de 8 bits para A.
    if (is_address_only(argv[1]) && is_a_register(argv[0]))
    {
        op = opcode->op1;
        out(REC_DATA, 0, 0, &op, 1);
        generate(argv[1]->right, 0, false);
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
    }
    // IN r8, (C): input from port (C) into an 8-bit register.
    // IN r8, (C): entrada da porta (C) para um registrador de 8 bits.
    else if (is_8bit_register(argv[0]) && tryget_reg(argv[1], REG_CMP))
    {
        reg_t *c_port = tryget_reg(argv[1], REG_CMP);
        if (c_port->value_aux != 3)
            error_expr(argv[1], "register C expected.");
        op = 0xed;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0x40 | (argv[0]->reg->value << 3);
        out(REC_DATA, 0, 0, &op, 1);
    }
    else
        error_expr(argv[0], "invalid arguments.");
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
        {"z", 1, 1, REG_CMP},
        {"nc", 2, 2, REG_CMP},
        //{"c",  3, 3, REG_CMP},
        {"po", 4, 4, REG_CMP},
        {"pe", 5, 5, REG_CMP},
        {"p", 6, 6, REG_CMP},
        {"m", 7, 7, REG_CMP},

        {"af'", 0, 0, REG_16BIT_ALT},

        {"i", 0, 0, REG_I},

        {"r", 0, 0, REG_R},

        {NULL, 0, 0, 0}};

opcode_t _prefix[] =
    {
        {"cb", 0xcb, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
        {"dd", 0xdd, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
        {"ddcb", 0xdd, 0xcb, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"ed", 0xed, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
        {"fd", 0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
        {"fdcb", 0xfd, 0xcb, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {NULL, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, NULL}};

opcode_t _opcode[] =
    {
        {"nop", 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
        {"djnz", 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, emit_offset8bit},
        {"jr", 0x20, 0x18, 0x00, 0x00, 0x00, 0x00, emit_cmp__offset8bit},
        {"inc", 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, emit_reg16bitsp_or_reg8bit_or_index},
        {"dec", 0x0b, 0x05, 0x00, 0x00, 0x00, 0x00, emit_reg16bitsp_or_reg8bit_or_index},
        {"rlca", 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
        {"rla", 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
        {"daa", 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
        {"scf", 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
        {"rrca", 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
        {"rra", 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
        {"cpl", 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
        {"ccf", 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},

        {"ld", 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, emit_ld},
        {"halt", 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
        {"add", 0x80, 0x09, 0xc6, 0x00, 0x00, 0x00, emit_logic},
        {"adc", 0x88, 0x4a, 0xce, 0xed, 0x00, 0x00, emit_logic},
        {"sub", 0x90, 0x00, 0xd6, 0x00, 0x00, 0x00, emit_logic},
        {"sbc", 0x98, 0x42, 0xde, 0xed, 0x00, 0x00, emit_logic},
        {"and", 0xa0, 0x00, 0xe6, 0x00, 0x00, 0x00, emit_logic},
        {"xor", 0xa8, 0x00, 0xee, 0x00, 0x00, 0x00, emit_logic},
        {"or", 0xb0, 0x00, 0xf6, 0x00, 0x00, 0x00, emit_logic},
        {"cp", 0xb8, 0x00, 0xfe, 0x00, 0x00, 0x00, emit_logic},

        {"ret", 0xc0, 0xc9, 0x00, 0x00, 0x00, 0x00, emit_cmp},
        {"pop", 0xc1, 0x00, 0x00, 0x00, 0x00, 0x00, emit_reg16bitaf_or_reg8bit_or_index},
        {"push", 0xc5, 0x00, 0x00, 0x00, 0x00, 0x00, emit_reg16bitaf_or_reg8bit_or_index},
        {"jp", 0xc2, 0xc3, 0x00, 0x00, 0x00, 0x00, emit_cmp__offset16bit},
        {"call", 0xc4, 0xcd, 0x00, 0x00, 0x00, 0x00, emit_cmp__offset16bit},
        {"rst", 0xc7, 0x00, 0x00, 0x00, 0x00, 0x00, emit_rst},
        {"ex", 0x08, 0xe3, 0xeb, 0x00, 0x00, 0x00, emit_ex},
        {"di", 0xf3, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
        {"ei", 0xfb, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
        {"exx", 0xd9, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
        {"out", 0xd3, 0x00, 0x00, 0x00, 0x00, 0x00, emit_out},
        {"in", 0xdb, 0x00, 0x00, 0x00, 0x00, 0x00, emit_in},

        {"rlc", 0x00, 0x00, 0x00, 0x00, 0xcb, 0xff, emit_logic},
        {"rrc", 0x08, 0x00, 0x00, 0x00, 0xcb, 0x00, emit_logic},
        {"rl", 0x10, 0x00, 0x00, 0x00, 0xcb, 0x00, emit_logic},
        {"rr", 0x18, 0x00, 0x00, 0x00, 0xcb, 0x00, emit_logic},
        {"sla", 0x20, 0x00, 0x00, 0x00, 0xcb, 0x00, emit_logic},
        {"sra", 0x28, 0x00, 0x00, 0x00, 0xcb, 0x00, emit_logic},
        {"sll", 0x30, 0x00, 0x00, 0x00, 0xcb, 0x00, emit_logic},
        {"srl", 0x38, 0x00, 0x00, 0x00, 0xcb, 0x00, emit_logic},
        {"bit", 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, emit_bit},
        {"res", 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, emit_bit},
        {"set", 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, emit_bit},

        {"ldi", 0xed, 0xa0, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"ldir", 0xed, 0xb0, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"cpi", 0xed, 0xa1, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"cpir", 0xed, 0xb1, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"ini", 0xed, 0xa2, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"inir", 0xed, 0xb2, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"outi", 0xed, 0xa3, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"otir", 0xed, 0xb3, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"ldd", 0xed, 0xa8, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"lddr", 0xed, 0xb8, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"cpd", 0xed, 0xa9, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"cpdr", 0xed, 0xb9, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"ind", 0xed, 0xaa, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"indr", 0xed, 0xba, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"outd", 0xed, 0xab, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"otdr", 0xed, 0xbb, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"reti", 0xed, 0x4d, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"rrd", 0xed, 0x67, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"rld", 0xed, 0x6f, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"retn", 0xed, 0x45, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"neg", 0xed, 0x44, 0x00, 0x00, 0x00, 0x00, emit_simple_2bytes},
        {"im", 0x46, 0x56, 0x5e, 0xed, 0x00, 0x00, emit_im},
        {NULL, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, NULL}};
