#include "../asm.h"

rectype_t _cpu = REC_CPU_8086;

enum
{
    REG_8BIT = 0x01,
    REG_16BIT = 0x02,
    REG_SEG = 0x04,
    REG_PTR = 0x08
};

enum
{
    CMP_O = 0,
    CMP_NO = 1,
    CMP_B = 2,
    CMP_NAE = 2,
    CMP_NB = 3,
    CMP_AE = 3,
    CMP_E = 4,
    CMP_Z = 4,
    CMP_NE = 5,
    CMP_NZ = 5,
    CMP_BE = 6,
    CMP_NA = 6,
    CMP_NBE = 7,
    CMP_A = 7,
    CMP_S = 8,
    CMP_NS = 9,
    CMP_P = 10,
    CMP_PE = 10,
    CMP_NP = 11,
    CMP_PO = 11,
    CMP_L = 12,
    CMP_NGE = 12,
    CMP_NL = 13,
    CMP_GE = 13,
    CMP_LE = 14,
    CMP_NG = 14,
    CMP_NLE = 15,
    CMP_G = 15
};

// Inverts a comparison condition by toggling the least significant bit.
// Inverte uma condicao de comparacao alternando o bit menos significativo.
uint8_t invert_comparsion(uint8_t cmp)
{
    if (cmp & 1)
        return cmp & 0xfe;
    else
        return cmp | 1;
}

// Checks if the argument is an 8-bit register.
// Verifica se o argumento eh um registrador de 8 bits.
static int is_reg_8bit(expr_t *arg)
{
    if (!arg)
        return false;
    if (arg->token == TOK_REGISTER && (arg->reg->group & REG_8BIT))
    {
        return true;
    }
    return false;
}

// Checks if the argument is a 16-bit register.
// Verifica se o argumento eh um registrador de 16 bits.
static int is_reg_16bit(expr_t *arg)
{
    if (!arg)
        return false;
    if (arg->token == TOK_REGISTER && (arg->reg->group & REG_16BIT))
    {
        return true;
    }
    return false;
}

// Checks if the argument is a segment register.
// Verifica se o argumento eh um registrador de segmento.
static int is_reg_seg(expr_t *arg)
{
    if (!arg)
        return false;
    if (arg->token == TOK_REGISTER && (arg->reg->group & REG_SEG))
    {
        return true;
    }
    return false;
}

// Checks if the argument is the 8-bit accumulator (AL).
// Verifica se o argumento eh o acumulador de 8 bits (AL).
static int is_acc_8bit(expr_t *arg)
{
    if (!arg)
        return false;
    if (arg->token == TOK_REGISTER && (arg->reg->group & REG_8BIT) && arg->reg->value == 0)
    {
        return true;
    }
    return false;
}

// Checks if the argument is the 16-bit accumulator (AX).
// Verifica se o argumento eh o acumulador de 16 bits (AX).
static int is_acc_16bit(expr_t *arg)
{
    if (!arg)
        return false;
    if (arg->token == TOK_REGISTER && (arg->reg->group & REG_16BIT) && arg->reg->value == 0)
    {
        return true;
    }
    return false;
}

// Checks if the argument is a constant value (not a register, mnemonic, or index expression).
// Verifica se o argumento eh um valor constante (nao eh registrador, mnemonic ou expressao indexada).
static bool is_value(expr_t *arg)
{
    if (arg->left && !is_value(arg->left))
        return false;
    if (arg->right && !is_value(arg->right))
        return false;
    return arg->token != TOK_REGISTER && arg->token != TOK_MNEMONIC && arg->token != TOK_INDEX_OPEN;
}

// Checks if the argument is a value or a pointer register (BX, BP, SI, DI).
// Verifica se o argumento eh um valor ou um registrador ponteiro (BX, BP, SI, DI).
static bool is_value_or_ptr(expr_t *arg)
{
    if (arg->left && !is_value_or_ptr(arg->left))
        return false;
    if (arg->right && !is_value_or_ptr(arg->right))
        return false;
    return (arg->token == TOK_REGISTER && (arg->reg->group & REG_PTR)) || (arg->token != TOK_REGISTER && arg->token != TOK_MNEMONIC && arg->token != TOK_INDEX_OPEN);
}

// Checks if the argument is a memory address expression (enclosed in brackets).
// Verifica se o argumento eh uma expressao de endereco de memoria (entre colchetes).
static bool is_address(expr_t *arg)
{
    if (arg->left && !is_value(arg->left))
        return false;
    if (arg->right && !is_value(arg->right))
        return false;
    return arg->token == TOK_INDEX_OPEN;
}

// Checks if the argument is a register-based memory address (e.g., [bx+si]).
// Verifica se o argumento eh um endereco de memoria baseado em registrador (ex: [bx+si]).
static bool is_reg_address(expr_t *arg)
{
    if (arg->token != TOK_INDEX_OPEN)
        return false;
    expr_t *e = arg->right;
    if (e->token == TOK_COLON && e->left && e->left->token == TOK_REGISTER && (e->left->reg->group & REG_SEG))
        e = e->right;
    return is_value_or_ptr(e);
}

// Gets the register from a ModRM expression, expecting a pointer register.
// Obtem o registrador de uma expressao ModRM, esperando um registrador ponteiro.
static reg_t *get_mrm_reg(expr_t *arg)
{
    if (arg->token != TOK_REGISTER || (arg->reg->group & REG_PTR) == 0)
        error_expr(arg, "pointer register expected [Token: %s]", arg->text);
    return arg->reg;
}

// Computes the ModRM byte for a memory/register operand expression.
// Calcula o byte ModRM para uma expressao de operando de memoria/registrador.
static int get_mrm(expr_t *arg)
{
    // Validate that the expression is an indexed memory operand
    // Valida se a expressao eh um operando de memoria indexada
    if (arg->token != TOK_INDEX_OPEN)
        error_expr(arg, "'[' expected");
    reg_t *reg1 = NULL;
    reg_t *reg2 = NULL;
    expr_t *e = arg->right;
    // Skip segment prefix (e.g., ds:)
    // Ignora prefixo de segmento (ex: ds:)
    if (e->token == TOK_COLON && e->left && e->left->token == TOK_REGISTER && (e->left->reg->group & REG_SEG))
        e = e->right;
    // Traverse the expression tree to find register(s)
    // Percorre a arvore de expressao para encontrar o(s) registrador(es)
    while (e)
    {
        // Handle a single pointer register (BX, BP, SI, DI)
        // Trata um unico registrador ponteiro (BX, BP, SI, DI)
        if (e->token == TOK_REGISTER)
        {
            if ((e->reg->group & REG_PTR) == 0)
                error_expr(e, "pointer register expected.");
            switch (e->reg->value)
            {
            case 3: // bx
                return 0b00000111;
                break;
            case 5: // bp
                return 0b00000110;
                // error_expr(arg, "invalid usage of bp register. 8086 only support [bp+XX].");
                break;
            case 6: // si
                return 0b00000100;
                break;
            case 7: // di
                return 0b00000101;
                break;
            default:
                error_expr(arg, "invalid pointer register [Token: %s]", arg->text);
                break;
            }
        }
        // Handle a two-register combination (e.g., bx+si)
        // Trata uma combinacao de dois registradores (ex: bx+si)
        else if (e->right && e->left && e->right->token == TOK_REGISTER && e->left->token == TOK_REGISTER)
        {
            if ((e->right->reg->group & REG_PTR) == 0)
                error_expr(e->right, "pointer register expected.");
            if ((e->left->reg->group & REG_PTR) == 0)
                error_expr(e->left, "pointer register expected.");
            if (e->token != TOK_ADD)
                error_expr(e, "'+' expected between pointer registers.");
            reg1 = e->right->reg;
            reg2 = e->left->reg;
            break;
        }
        e = e->left;
    }
    // Encode the ModRM byte for two-register combinations
    // Codifica o byte ModRM para combinacoes de dois registradores
    if (reg1 && reg2)
    {
        switch (reg1->value)
        {
        case 3: // bx
            switch (reg2->value)
            {
            case 6: // si
                return 0b00000000;
                break;
            case 7: // di
                return 0b00000001;
                break;
            default:
                error_expr(arg, "invalid pointer register");
                break;
            }
            break;
        case 5: // bp
            switch (reg2->value)
            {
            case 6: // si
                return 0b00000010;
                break;
            case 7: // di
                return 0b00000011;
                break;
            default:
                error_expr(arg, "invalid pointer register");
                break;
            }
            break;
        case 6: // si
            switch (reg2->value)
            {
            case 3: // bx
                return 0b00000000;
                break;
            case 5: // bp
                return 0b00000010;
                break;
            default:
                error_expr(arg, "invalid pointer register");
                break;
            }
            break;
        case 7: // di
            switch (reg2->value)
            {
            case 3: // bx
                return 0b00000001;
                break;
            case 5: // bp
                return 0b00000011;
                break;
            default:
                error_expr(arg, "invalid pointer register");
                break;
            }
            break;
        default:
            error_expr(arg, "invalid pointer register");
            break;
        }
    }
    // No valid register found; report error
    // Nenhum registrador valido encontrado; reporta erro
    error_expr(arg, "pointer register expected [Token: %s]", arg->text);
    return 0;
}

// Emits a segment override prefix byte if a segment register is present in any operand.
// Emite um byte de prefixo de override de segmento se um registrador de segmento estiver presente em algum operando.
static void emit_seg_prefix(int argc, expr_t *argv[])
{
    for (int i = 0; i < argc; i++)
    {
        if (argv[i]->token == TOK_INDEX_OPEN && argv[i]->right->token == TOK_COLON &&
            argv[i]->right->left && argv[i]->right->left->token == TOK_REGISTER &&
            (argv[i]->right->left->reg->group & REG_SEG))
        {
            static const uint8_t seg_ops[] = {0x26, 0x2E, 0x36, 0x3E};
            uint8_t op = seg_ops[argv[i]->right->left->reg->value];
            out(REC_DATA, 0, 0, &op, 1);
            expr_t *colon = argv[i]->right;
            argv[i]->right = colon->right;
            colon->right = NULL;
            free_expr(colon);
        }
    }
}

// Emits a simple single-byte opcode with no operands.
// Emite um opcode simples de um byte sem operandos.
static void emit_simple(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    validate(mnemonic, false, false, false, false);
    if (argc != 0)
        error_expr(mnemonic, "invalid argument count.");
    out(REC_DATA, 0, 0, &opcode->op1, 1);
}

// Emits a two-byte opcode with no operands.
// Emite um opcode de dois bytes sem operandos.
static void emit_simple_2bytes(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    validate(mnemonic, false, false, false, false);
    if (argc != 0)
        error_expr(mnemonic, "invalid argument count.");
    out(REC_DATA, 0, 0, &opcode->op1, 1);
    out(REC_DATA, 0, 0, &opcode->op2, 1);
}

// Emits ModRM-encoded instructions with register/memory and immediate operands (basic form).
// Emite instrucoes codificadas em ModRM com operandos de registrador/memoria e imediato (forma basica).
static void emit_mrm_simple(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    bool include_value = false;
    uint8_t op = 0;
    if (argc != 2)
        error_expr(mnemonic, "invalid argument count.");
    emit_seg_prefix(argc, argv);
    // op1: Reg/Mem with Reg to Reg
    // op2 op3: Imm to Reg/Memory
    // op4: Imm to Acc
    // Handle accumulator with immediate value (8-bit)
    // Trata acumulador com valor imediato (8 bits)
    if (is_acc_8bit(argv[0]) && is_value(argv[1]))
    {
        validate(mnemonic, true, false, false, false);
        op = opcode->op4;
        out(REC_DATA, 0, 0, &op, 1);
        generate(argv[1], 1, false);
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
    }
    // Handle accumulator with immediate value (16-bit)
    // Trata acumulador com valor imediato (16 bits)
    else if (is_acc_16bit(argv[0]) && is_value(argv[1]))
    {
        validate(mnemonic, false, true, false, false);
        op = opcode->op4 | 1;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[1], 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Handle 8-bit register with immediate value
    // Trata registrador de 8 bits com valor imediato
    else if (is_reg_8bit(argv[0]) && is_value(argv[1]))
    {
        validate(mnemonic, true, false, false, false);
        op = opcode->op2;
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op3 | 0b11000000 | argv[0]->reg->value;
        out(REC_DATA, 0, 0, &op, 1);
        generate(argv[1], 1, false);
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
    }
    // Handle 16-bit register with immediate value
    // Trata registrador de 16 bits com valor imediato
    else if (is_reg_16bit(argv[0]) && is_value(argv[1]))
    {
        validate(mnemonic, false, true, false, false);
        op = opcode->op2 | 1;
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op3 | 0b11000000 | argv[0]->reg->value;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[1], 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Handle 8-bit register to 8-bit register
    // Trata registrador de 8 bits para registrador de 8 bits
    else if (is_reg_8bit(argv[0]) && is_reg_8bit(argv[1]))
    {
        validate(mnemonic, true, false, false, false);
        op = opcode->op1;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0b11000000 | argv[0]->reg->value | (argv[1]->reg->value << 3);
        out(REC_DATA, 0, 0, &op, 1);
    }
    // Handle 16-bit register to 16-bit register
    // Trata registrador de 16 bits para registrador de 16 bits
    else if (is_reg_16bit(argv[0]) && is_reg_16bit(argv[1]))
    {
        validate(mnemonic, false, true, false, false);
        op = opcode->op1 | 1;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0b11000000 | argv[0]->reg->value | (argv[1]->reg->value << 3);
        out(REC_DATA, 0, 0, &op, 1);
    }
    // Handle memory address with register operand
    // Trata endereco de memoria com operando registrador
    else if (is_address(argv[0]) && (is_reg_8bit(argv[1]) || is_reg_16bit(argv[1])))
    {
        if (is_reg_8bit(argv[1]))
            validate(mnemonic, true, false, false, false);
        else
            validate(mnemonic, false, true, false, false);
        op = opcode->op1 | (is_reg_16bit(argv[1]) ? 1 : 0);
        out(REC_DATA, 0, 0, &op, 1);
        op = 0b00000110 | (argv[1]->reg->value << 3);
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Handle register with memory address operand
    // Trata registrador com operando de endereco de memoria
    else if ((is_reg_8bit(argv[0]) || is_reg_16bit(argv[0])) && is_address(argv[1]))
    {
        if (is_reg_8bit(argv[0]))
            validate(mnemonic, true, false, false, false);
        else
            validate(mnemonic, false, true, false, false);
        op = opcode->op1 | 2 | (is_reg_16bit(argv[0]) ? 1 : 0);
        out(REC_DATA, 0, 0, &op, 1);
        op = 0b00000110 | (argv[0]->reg->value << 3);
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[1]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Handle register-based memory address with register operand
    // Trata endereco de memoria baseado em registrador com operando registrador
    else if (is_reg_address(argv[0]) && (is_reg_8bit(argv[1]) || is_reg_16bit(argv[1])))
    {
        if (is_reg_8bit(argv[1]))
            validate(mnemonic, true, false, false, false);
        else
            validate(mnemonic, false, true, false, false);
        op = opcode->op1 | (is_reg_16bit(argv[1]) ? 1 : 0);
        out(REC_DATA, 0, 0, &op, 1);
        op = get_mrm(argv[0]) | (argv[1]->reg->value << 3);
        argv[0] = optimize(filter_registers(argv[0]));
        if (!(argv[0]->right->token == TOK_VALUE && argv[0]->right->value == 0 && (op & 0x7) != 0b110))
        {
            op |= 0b10000000;
            include_value = true;
        }
        out(REC_DATA, 0, 0, &op, 1);
        if (include_value)
            out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Handle register with register-based memory address operand
    // Trata registrador com operando de endereco de memoria baseado em registrador
    else if ((is_reg_8bit(argv[0]) || is_reg_16bit(argv[0])) && is_reg_address(argv[1]))
    {
        if (is_reg_8bit(argv[0]))
            validate(mnemonic, true, false, false, false);
        else
            validate(mnemonic, false, true, false, false);
        op = opcode->op1 | 2 | (is_reg_16bit(argv[0]) ? 1 : 0);
        out(REC_DATA, 0, 0, &op, 1);
        op = get_mrm(argv[1]) | (argv[0]->reg->value << 3);
        argv[1] = optimize(filter_registers(argv[1]));
        if (!(argv[1]->right->token == TOK_VALUE && argv[1]->right->value == 0 && (op & 0x7) != 0b110))
        {
            op |= 0b10000000;
            include_value = true;
        }
        out(REC_DATA, 0, 0, &op, 1);
        if (include_value)
            out(generate(argv[1]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    else
        error_expr(mnemonic, "invalid arguments");
}

// Emits ModRM-encoded instructions for all operand combinations including segment registers.
// Emite instrucoes codificadas em ModRM para todas as combinacoes de operandos, incluindo registradores de segmento.
static void emit_mrm_complete(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    bool include_value = false;
    uint8_t op = 0;
    if (argc != 2)
        error_expr(mnemonic, "invalid argument count.");
    emit_seg_prefix(argc, argv);
    // op1: Reg/Mem with Reg to Reg
    // op2 op3: Imm to Reg/Memory
    // op4: Imm to Acc
    // Handle accumulator with memory address (8-bit)
    // Trata acumulador com endereco de memoria (8 bits)
    if (is_acc_8bit(argv[0]) && is_address(argv[1]))
    {
        validate(mnemonic, true, false, false, false);
        op = opcode->op4;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[1]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Handle accumulator with memory address (16-bit)
    // Trata acumulador com endereco de memoria (16 bits)
    else if (is_acc_16bit(argv[0]) && is_address(argv[1]))
    {
        validate(mnemonic, false, true, false, false);
        op = opcode->op4 | 1;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[1]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Handle 8-bit register with immediate value
    // Trata registrador de 8 bits com valor imediato
    else if (is_reg_8bit(argv[0]) && is_value(argv[1]))
    {
        validate(mnemonic, true, false, false, false);
        op = opcode->op6 | argv[0]->reg->value;
        out(REC_DATA, 0, 0, &op, 1);
        generate(argv[1], 1, false);
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
    }
    // Handle 16-bit register with immediate value
    // Trata registrador de 16 bits com valor imediato
    else if (is_reg_16bit(argv[0]) && is_value(argv[1]))
    {
        validate(mnemonic, false, true, false, false);
        op = opcode->op6 | 0b1000 | argv[0]->reg->value;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[1], 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Handle 8-bit register to 8-bit register
    // Trata registrador de 8 bits para registrador de 8 bits
    else if (is_reg_8bit(argv[0]) && is_reg_8bit(argv[1]))
    {
        validate(mnemonic, true, false, false, false);
        op = opcode->op1;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0b11000000 | argv[0]->reg->value | (argv[1]->reg->value << 3);
        out(REC_DATA, 0, 0, &op, 1);
    }
    // Handle 16-bit register to 16-bit register
    // Trata registrador de 16 bits para registrador de 16 bits
    else if (is_reg_16bit(argv[0]) && is_reg_16bit(argv[1]))
    {
        validate(mnemonic, false, true, false, false);
        op = opcode->op1 | 1;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0b11000000 | argv[0]->reg->value | (argv[1]->reg->value << 3);
        out(REC_DATA, 0, 0, &op, 1);
    }
    // Handle memory address with register operand
    // Trata endereco de memoria com operando registrador
    else if (is_address(argv[0]) && (is_reg_8bit(argv[1]) || is_reg_16bit(argv[1])))
    {
        if (is_reg_8bit(argv[1]))
            validate(mnemonic, true, false, false, false);
        else
            validate(mnemonic, false, true, false, false);
        op = opcode->op1 | (is_reg_16bit(argv[1]) ? 1 : 0);
        out(REC_DATA, 0, 0, &op, 1);
        op = 0b00000110 | (argv[1]->reg->value << 3);
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Handle register with memory address operand
    // Trata registrador com operando de endereco de memoria
    else if ((is_reg_8bit(argv[0]) || is_reg_16bit(argv[0])) && is_address(argv[1]))
    {
        if (is_reg_8bit(argv[0]))
            validate(mnemonic, true, false, false, false);
        else
            validate(mnemonic, false, true, false, false);
        op = opcode->op1 | 2 | (is_reg_16bit(argv[0]) ? 1 : 0);
        out(REC_DATA, 0, 0, &op, 1);
        op = 0b00000110 | (argv[0]->reg->value << 3);
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[1]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Handle register-based memory address with register operand
    // Trata endereco de memoria baseado em registrador com operando registrador
    else if (is_reg_address(argv[0]) && (is_reg_8bit(argv[1]) || is_reg_16bit(argv[1])))
    {
        if (is_reg_8bit(argv[1]))
            validate(mnemonic, true, false, false, false);
        else
            validate(mnemonic, false, true, false, false);
        op = opcode->op1 | (is_reg_16bit(argv[1]) ? 1 : 0);
        out(REC_DATA, 0, 0, &op, 1);
        op = get_mrm(argv[0]) | (argv[1]->reg->value << 3);
        argv[0] = optimize(filter_registers(argv[0]));
        if (!(argv[0]->right->token == TOK_VALUE && argv[0]->right->value == 0 && (op & 0x7) != 0b110))
        {
            op |= 0b10000000;
            include_value = true;
        }
        out(REC_DATA, 0, 0, &op, 1);
        if (include_value)
            out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Handle register with register-based memory address operand
    // Trata registrador com operando de endereco de memoria baseado em registrador
    else if ((is_reg_8bit(argv[0]) || is_reg_16bit(argv[0])) && is_reg_address(argv[1]))
    {
        if (is_reg_8bit(argv[0]))
            validate(mnemonic, true, false, false, false);
        else
            validate(mnemonic, false, true, false, false);
        op = opcode->op1 | 2 | (is_reg_16bit(argv[0]) ? 1 : 0);
        out(REC_DATA, 0, 0, &op, 1);
        op = get_mrm(argv[1]) | (argv[0]->reg->value << 3);
        argv[1] = optimize(filter_registers(argv[1]));
        if (!(argv[1]->right->token == TOK_VALUE && argv[1]->right->value == 0 && (op & 0x7) != 0b110))
        {
            op |= 0b10000000;
            include_value = true;
        }
        out(REC_DATA, 0, 0, &op, 1);
        if (include_value)
            out(generate(argv[1]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Handle 16-bit register to segment register
    // Trata registrador de 16 bits para registrador de segmento
    else if (is_reg_16bit(argv[0]) && is_reg_seg(argv[1]))
    {
        validate(mnemonic, false, true, false, false);
        op = opcode->op5;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0b11000000 | (argv[1]->reg->value << 3) | argv[0]->reg->value;
        out(REC_DATA, 0, 0, &op, 1);
    }
    // Handle segment register to 16-bit register
    // Trata registrador de segmento para registrador de 16 bits
    else if (is_reg_seg(argv[0]) && is_reg_16bit(argv[1]))
    {
        validate(mnemonic, false, true, false, false);
        op = opcode->op5 | 2;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0b11000000 | (argv[0]->reg->value << 3) | argv[1]->reg->value;
        out(REC_DATA, 0, 0, &op, 1);
    }
    // Handle memory address to segment register
    // Trata endereco de memoria para registrador de segmento
    else if (is_address(argv[0]) && is_reg_seg(argv[1]))
    {
        validate(mnemonic, false, true, false, false);
        op = opcode->op5;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0b00000110 | (argv[1]->reg->value << 3);
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Handle segment register to memory address
    // Trata registrador de segmento para endereco de memoria
    else if (is_reg_seg(argv[0]) && is_address(argv[1]))
    {
        validate(mnemonic, false, true, false, false);
        op = opcode->op5 | 2;
        out(REC_DATA, 0, 0, &op, 1);
        op = 0b00000110 | (argv[0]->reg->value << 3);
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[1]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Handle segment register to register-based memory address
    // Trata registrador de segmento para endereco de memoria baseado em registrador
    else if (is_reg_seg(argv[0]) && is_reg_address(argv[1]))
    {
        validate(mnemonic, false, true, false, false);
        op = opcode->op5 | 2;
        out(REC_DATA, 0, 0, &op, 1);
        op = get_mrm(argv[1]) | (argv[0]->reg->value << 3);
        argv[1] = optimize(filter_registers(argv[1]));
        if (!(argv[1]->right->token == TOK_VALUE && argv[1]->right->value == 0 && (op & 0x7) != 0b110))
        {
            op |= 0b10000000;
            include_value = true;
        }
        out(REC_DATA, 0, 0, &op, 1);
        if (include_value)
            out(generate(argv[1]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Handle register-based memory address to segment register
    // Trata endereco de memoria baseado em registrador para registrador de segmento
    else if (is_reg_address(argv[0]) && is_reg_seg(argv[1]))
    {
        validate(mnemonic, false, true, false, false);
        op = opcode->op5;
        out(REC_DATA, 0, 0, &op, 1);
        op = get_mrm(argv[0]) | (argv[1]->reg->value << 3);
        argv[0] = optimize(filter_registers(argv[0]));
        if (!(argv[0]->right->token == TOK_VALUE && argv[0]->right->value == 0 && (op & 0x7) != 0b110))
        {
            op |= 0b10000000;
            include_value = true;
        }
        out(REC_DATA, 0, 0, &op, 1);
        if (include_value)
            out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Handle memory address with immediate value
    // Trata endereco de memoria com valor imediato
    else if (is_address(argv[0]) && is_value(argv[1]))
    {
        if (mnemonic->force_byte)
            validate(mnemonic, true, false, false, false);
        else
            validate(mnemonic, false, true, false, false);
        op = opcode->op2 | (mnemonic->force_word || (!mnemonic->force_byte) ? 1 : 0);
        out(REC_DATA, 0, 0, &op, 1);
        op = 0b00000110; /* mod=00, reg=000, rm=110 (direct address) */
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        if (mnemonic->force_word || (!mnemonic->force_byte))
            out(generate(argv[1], 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        else
        {
            generate(argv[1], 1, false);
            out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
        }
    }
    // Handle register-based memory address with immediate value
    // Trata endereco de memoria baseado em registrador com valor imediato
    else if (is_reg_address(argv[0]) && is_value(argv[1]))
    {
        if (mnemonic->force_byte)
            validate(mnemonic, true, false, false, false);
        else
            validate(mnemonic, false, true, false, false);
        op = opcode->op2 | (mnemonic->force_word || (!mnemonic->force_byte) ? 1 : 0);
        out(REC_DATA, 0, 0, &op, 1);
        op = get_mrm(argv[0]);
        argv[0] = optimize(filter_registers(argv[0]));
        if (!(argv[0]->right->token == TOK_VALUE && argv[0]->right->value == 0 && (op & 0x7) != 0b110))
        {
            op |= 0b10000000;
            include_value = true;
        }
        out(REC_DATA, 0, 0, &op, 1);
        if (include_value)
            out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        if (mnemonic->force_word || (!mnemonic->force_byte))
            out(generate(argv[1], 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        else
        {
            generate(argv[1], 1, false);
            out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
        }
    }
    else
        error_expr(mnemonic, "invalid arguments");
}

// Emits instructions that embed a 16-bit register in the opcode or use a single ModRM operand.
// Emite instrucoes que embutem um registrador de 16 bits no opcode ou usam um unico operando ModRM.
static void emit_embbed_reg16bit_or_single_mrm(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    bool include_value = false;
    uint8_t op = opcode->op1;
    if (argc != 1)
        error_expr(mnemonic, "invalid argument count.");
    emit_seg_prefix(argc, argv);
    // Handle 16-bit register operand (embedded in opcode)
    // Trata operando de registrador de 16 bits (embutido no opcode)
    if (is_reg_16bit(argv[0]))
    {
        validate(mnemonic, false, true, false, false);
        op |= argv[0]->reg->value & 0x7;
        out(REC_DATA, 0, 0, &op, 1);
    }
    // Handle 8-bit register operand (ModRM encoding)
    // Trata operando de registrador de 8 bits (codificacao ModRM)
    else if (is_reg_8bit(argv[0]))
    {
        validate(mnemonic, true, false, false, false);
        op = opcode->op2;
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op3 | argv[0]->reg->value | 0b11000000;
        out(REC_DATA, 0, 0, &op, 1);
    }
    // Handle direct memory address operand
    // Trata operando de endereco de memoria direto
    else if (is_address(argv[0]))
    {
        validate(mnemonic, true, true, false, false);
        if (!mnemonic->force_byte && !mnemonic->force_word)
            error_expr(mnemonic, "pointer size not defined.");
        op = opcode->op2 | (mnemonic->force_word ? 1 : 0);
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op3 | 0b110;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Handle register-based memory address operand
    // Trata operando de endereco de memoria baseado em registrador
    else
    {
        validate(mnemonic, true, true, false, false);
        if (!mnemonic->force_byte && !mnemonic->force_word)
            error_expr(mnemonic, "pointer size not defined.");
        op = opcode->op2 | (mnemonic->force_word ? 1 : 0);
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op3 | get_mrm(argv[0]);
        argv[0] = optimize(filter_registers(argv[0]));
        if (!(argv[0]->right->token == TOK_VALUE && argv[0]->right->value == 0 && (op & 0x7) != 0b110))
        {
            op |= 0b10000000;
            include_value = true;
        }
        out(REC_DATA, 0, 0, &op, 1);
        if (include_value)
            out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
}

// Emits instructions with a single ModRM operand (unary operations like NOT, NEG, DIV, MUL).
// Emite instrucoes com um unico operando ModRM (operacoes unarias como NOT, NEG, DIV, MUL).
static void emit_single_mrm(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    bool include_value = false;
    uint8_t op = opcode->op1;
    if (argc != 1)
        error_expr(mnemonic, "invalid argument count.");
    emit_seg_prefix(argc, argv);
    // Handle 8-bit register operand
    // Trata operando de registrador de 8 bits
    if (is_reg_8bit(argv[0]))
    {
        validate(mnemonic, true, false, false, false);
        op = opcode->op1;
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op2 | argv[0]->reg->value | 0b11000000;
        out(REC_DATA, 0, 0, &op, 1);
    }
    // Handle 16-bit register operand
    // Trata operando de registrador de 16 bits
    else if (is_reg_16bit(argv[0]))
    {
        validate(mnemonic, true, false, false, false);
        op = opcode->op1 | 1;
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op2 | argv[0]->reg->value | 0b11000000;
        out(REC_DATA, 0, 0, &op, 1);
    }
    // Handle direct memory address operand
    // Trata operando de endereco de memoria direto
    else if (is_address(argv[0]))
    {
        validate(mnemonic, true, true, false, false);
        if (!mnemonic->force_byte && !mnemonic->force_word)
            error_expr(mnemonic, "pointer size not defined.");
        op = opcode->op1 | (mnemonic->force_word ? 1 : 0);
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op2 | 0b110;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Handle register-based memory address operand
    // Trata operando de endereco de memoria baseado em registrador
    else
    {
        validate(mnemonic, true, true, false, false);
        if (!mnemonic->force_byte && !mnemonic->force_word)
            error_expr(mnemonic, "pointer size not defined.");
        op = opcode->op1 | (mnemonic->force_word ? 1 : 0);
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op2 | get_mrm(argv[0]);
        argv[0] = optimize(filter_registers(argv[0]));
        if (!(argv[0]->right->token == TOK_VALUE && argv[0]->right->value == 0 && (op & 0x7) != 0b110))
        {
            op |= 0b10000000;
            include_value = true;
        }
        out(REC_DATA, 0, 0, &op, 1);
        if (include_value)
            out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
}

// Emits instructions requiring a 16-bit accumulator and a single ModRM operand (e.g., LDS, LES, LEA).
// Emite instrucoes que requerem um acumulador de 16 bits e um unico operando ModRM (ex: LDS, LES, LEA).
static void emit_reg16bit_single_mrm(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    bool include_value = false;
    uint8_t op = opcode->op1;
    if (argc != 2)
        error_expr(mnemonic, "invalid argument count.");
    emit_seg_prefix(argc, argv);
    if (!is_acc_16bit(argv[0]))
        error_expr(argv[0], "16 bit register expected.");
    // Handle direct memory address operand
    // Trata operando de endereco de memoria direto
    if (is_address(argv[1]))
    {
        validate(mnemonic, false, false, false, false);
        op = opcode->op1;
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op2 | 0b110 | (argv[0]->reg->value << 3);
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[1]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Handle register-based memory address operand
    // Trata operando de endereco de memoria baseado em registrador
    else
    {
        validate(mnemonic, false, false, false, false);
        op = opcode->op1;
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op2 | get_mrm(argv[1]) | (argv[0]->reg->value << 3);
        argv[1] = optimize(filter_registers(argv[1]));
        if (!(argv[1]->right->token == TOK_VALUE && argv[1]->right->value == 0 && (op & 0x7) != 0b110))
        {
            op |= 0b10000000;
            include_value = true;
        }
        out(REC_DATA, 0, 0, &op, 1);
        if (include_value)
            out(generate(argv[1]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
}

// Emits the IN instruction (input from port).
// Emite a instrucao IN (entrada da porta).
static void emit_input(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op;
    validate(mnemonic, false, false, false, false);
    if (argc != 2)
        error_expr(mnemonic, "invalid argument count.");
    // Handle 8-bit accumulator (AL) with DX or immediate port
    // Trata acumulador de 8 bits (AL) com DX ou porta imediata
    if (is_acc_8bit(argv[0]))
    {
        // Input from DX port
        // Entrada da porta DX
        if (is_reg_16bit(argv[1]) && argv[1]->reg->value == 2)
        {
            out(REC_DATA, 0, 0, &opcode->op2, 1);
            return;
        }
        // Input from immediate port
        // Entrada da porta imediata
        else if (is_value(argv[1]))
        {
            op = opcode->op1;
            out(REC_DATA, 0, 0, &op, 1);
            generate(argv[1], 1, false);
            out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
            return;
        }
    }
    // Handle 16-bit accumulator (AX) with DX or immediate port
    // Trata acumulador de 16 bits (AX) com DX ou porta imediata
    else if (is_acc_16bit(argv[0]))
    {
        // Input from DX port
        // Entrada da porta DX
        if (is_reg_16bit(argv[1]) && argv[1]->reg->value == 2)
        {
            out(REC_DATA, 0, 0, &opcode->op2, 1);
            return;
        }
        // Input from immediate port
        // Entrada da porta imediata
        else if (is_value(argv[1]))
        {
            op = opcode->op1 | 1;
            out(REC_DATA, 0, 0, &op, 1);
            generate(argv[1], 1, false);
            out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
            return;
        }
    }
    error_expr(argv[0], "invalid arguments.");
}

// Emits the OUT instruction (output to port).
// Emite a instrucao OUT (saida para a porta).
static void emit_output(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op;
    validate(mnemonic, false, false, false, false);
    if (argc != 2)
        error_expr(mnemonic, "invalid argument count.");
    // Handle 8-bit accumulator (AL) with DX or immediate port
    // Trata acumulador de 8 bits (AL) com DX ou porta imediata
    if (is_acc_8bit(argv[1]))
    {
        // Output to DX port
        // Saida para a porta DX
        if (is_reg_16bit(argv[0]) && argv[0]->reg->value == 2)
        {
            out(REC_DATA, 0, 0, &opcode->op2, 1);
            return;
        }
        // Output to immediate port
        // Saida para porta imediata
        else if (is_value(argv[0]))
        {
            op = opcode->op1;
            out(REC_DATA, 0, 0, &op, 1);
            generate(argv[0], 1, false);
            out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
            return;
        }
    }
    // Handle 16-bit accumulator (AX) with DX or immediate port
    // Trata acumulador de 16 bits (AX) com DX ou porta imediata
    else if (is_acc_16bit(argv[1]))
    {
        // Output to DX port
        // Saida para a porta DX
        if (is_reg_16bit(argv[0]) && argv[0]->reg->value == 2)
        {
            out(REC_DATA, 0, 0, &opcode->op2, 1);
            return;
        }
        // Output to immediate port
        // Saida para porta imediata
        else if (is_value(argv[0]))
        {
            op = opcode->op1 | 1;
            out(REC_DATA, 0, 0, &op, 1);
            generate(argv[0], 1, false);
            out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
            return;
        }
    }
    error_expr(argv[0], "invalid arguments.");
}

// Emits the ESC instruction (Escape to coprocessor / 8087 FPU).
// Emite a instrucao ESC (Escape para coprocessador / 8087 FPU).
static void emit_esc(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    bool include_value = false;
    uint8_t op;
    if (argc != 2)
        error_expr(mnemonic, "invalid argument count.");
    emit_seg_prefix(argc, argv);
    // First argument must be a 3-bit immediate value (0-7)
    // Primeiro argumento deve ser um valor imediato de 3 bits (0-7)
    if (!is_value(argv[0]))
        error_expr(argv[0], "immediate value expected.");
    if (argv[0]->value > 7)
        error_expr(argv[0], "coprocessor opcode must be 0-7.");
    op = 0xD8 | (argv[0]->value & 0x07);
    // Second argument: register or memory operand
    // Segundo argumento: operando de registrador ou memoria
    if (is_reg_16bit(argv[1]))
    {
        validate(mnemonic, false, false, false, false);
        out(REC_DATA, 0, 0, &op, 1);
        op = 0b11000000 | argv[1]->reg->value;
        out(REC_DATA, 0, 0, &op, 1);
    }
    else if (is_address(argv[1]))
    {
        validate(mnemonic, false, false, false, false);
        out(REC_DATA, 0, 0, &op, 1);
        op = 0b00000110;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[1]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    else if (is_reg_address(argv[1]))
    {
        validate(mnemonic, false, false, false, false);
        out(REC_DATA, 0, 0, &op, 1);
        op = get_mrm(argv[1]);
        argv[1] = optimize(filter_registers(argv[1]));
        if (!(argv[1]->right->token == TOK_VALUE && argv[1]->right->value == 0 && (op & 0x7) != 0b110))
        {
            op |= 0b10000000;
            include_value = true;
        }
        out(REC_DATA, 0, 0, &op, 1);
        if (include_value)
            out(generate(argv[1]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    else
        error_expr(argv[1], "register or memory operand expected.");
}

// Emits the INT instruction (software interrupt).
// Emite a instrucao INT (interrupcao por software).
static void emit_int(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op = opcode->op1;
    validate(mnemonic, false, false, false, false);
    if (argc != 1)
        error_expr(mnemonic, "invalid argument count.");
    if (!is_value(argv[0]))
        error_expr(argv[0], "interrupt value expected.");
    if (argv[0]->token == TOK_VALUE && argv[0]->value == 3)
    {
        out(REC_DATA, 0, 0, &op, 1);
    }
    else
    {
        op |= 1;
        out(REC_DATA, 0, 0, &op, 1);
        generate(argv[0], 1, false);
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
    }
}

// Emits PUSH or POP instructions for registers or memory operands.
// Emite instrucoes PUSH ou POP para registradores ou operandos de memoria.
static void emit_push_pop(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    bool include_value = false;
    uint8_t op = opcode->op1;
    if (argc != 1)
        error_expr(mnemonic, "invalid argument count.");
    emit_seg_prefix(argc, argv);
    // Handle 16-bit general-purpose register
    // Trata registrador de uso geral de 16 bits
    if (is_reg_16bit(argv[0]))
    {
        validate(mnemonic, false, true, false, false);
        op |= argv[0]->reg->value & 0x7;
        out(REC_DATA, 0, 0, &op, 1);
    }
    // Handle segment register
    // Trata registrador de segmento
    else if (is_reg_seg(argv[0]))
    {
        validate(mnemonic, false, true, false, false);
        op = opcode->op4 | (argv[0]->reg->value << 3);
        out(REC_DATA, 0, 0, &op, 1);
    }
    // Handle direct memory address
    // Trata endereco de memoria direto
    else if (is_address(argv[0]))
    {
        validate(mnemonic, true, true, false, false);
        if (!mnemonic->force_byte && !mnemonic->force_word)
            error_expr(mnemonic, "pointer size not defined.");
        op = opcode->op2 | (mnemonic->force_word ? 1 : 0);
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op3 | 0b110;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Handle register-based memory address
    // Trata endereco de memoria baseado em registrador
    else
    {
        validate(mnemonic, true, true, false, false);
        if (!mnemonic->force_byte && !mnemonic->force_word)
            error_expr(mnemonic, "pointer size not defined.");
        op = opcode->op2 | (mnemonic->force_word ? 1 : 0);
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op3 | get_mrm(argv[0]);
        argv[0] = optimize(filter_registers(argv[0]));
        if (!(argv[0]->right->token == TOK_VALUE && argv[0]->right->value == 0 && (op & 0x7) != 0b110))
        {
            op |= 0b10000000;
            include_value = true;
        }
        out(REC_DATA, 0, 0, &op, 1);
        if (include_value)
            out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
}

// Emits shift/rotate instructions (RCL, RCR, ROL, ROR, SAL, SAR, SHL, SHR).
// Emite instrucoes de deslocamento/rotacao (RCL, RCR, ROL, ROR, SAL, SAR, SHL, SHR).
static void emit_shift_rotate(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    bool include_value = false;
    bool include_op2 = false;
    uint8_t op = opcode->op1;
    uint8_t op2 = opcode->op2;
    if (argc != 2)
        error_expr(mnemonic, "invalid argument count.");
    emit_seg_prefix(argc, argv);
    // Determine the operand type: 8-bit register, 16-bit register, direct address, or register address
    // Determina o tipo do operando: registrador de 8 bits, 16 bits, endereco direto ou endereco de registrador
    if (is_reg_8bit(argv[0]))
    {
        validate(mnemonic, true, false, false, false);
        op |= 0;
        op2 |= 0b11000000 | argv[0]->reg->value;
    }
    else if (is_reg_16bit(argv[0]))
    {
        validate(mnemonic, false, true, false, false);
        op |= 1;
        op2 |= 0b11000000 | argv[0]->reg->value;
    }
    else if (is_address(argv[0]))
    {
        validate(mnemonic, true, true, false, false);
        if (!mnemonic->force_byte && !mnemonic->force_word)
            error_expr(mnemonic, "pointer size not defined.");
        op |= mnemonic->force_word ? 1 : 0;
        op2 |= 0b10000110;
        include_value = true;
    }
    else if (is_reg_address(argv[0]))
    {
        validate(mnemonic, true, true, false, false);
        if (!mnemonic->force_byte && !mnemonic->force_word)
            error_expr(mnemonic, "pointer size not defined.");
        op |= mnemonic->force_word ? 1 : 0;
        op2 |= get_mrm(argv[0]);
        argv[0] = optimize(filter_registers(argv[0]));
        if (!(argv[0]->right->token == TOK_VALUE && argv[0]->right->value == 0 && (op & 0x7) != 0b110))
        {
            op2 |= 0b10000000;
            include_value = true;
        }
    }
    else
        error_expr(argv[0], "invalid arguments.");
    // Handle shift/rotate count: 1 (implied) or CL register
    // Trata contagem de deslocamento/rotacao: 1 (implicito) ou registrador CL
    if (is_value(argv[1]) && argv[1]->value == 1)
    {
        op |= 0;
    }
    else if (is_reg_8bit(argv[1]) && argv[1]->reg->value == 1)
    {
        op |= 2;
    }
    else
        error_expr(argv[1], "invalid arguments. [Supported values: 1, cl]");
    out(REC_DATA, 0, 0, &op, 1);
    out(REC_DATA, 0, 0, &op2, 1);
    if (include_value)
        out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
}

// Emits the RET instruction (return from subroutine).
// Emite a instrucao RET (retorno de sub-rotina).
static void emit_ret(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    bool include_value = false;
    validate_distance(mnemonic, false, false, true);
    uint8_t op = mnemonic->force_far ? opcode->op2 : opcode->op1;
    // Return with no arguments (near or far)
    // Retorno sem argumentos (proximo ou distante)
    if (argc == 0)
    {
        op |= 1;
        out(REC_DATA, 0, 0, &op, 1);
    }
    // Return with stack adjustment value
    // Retorno com valor de ajuste de pilha
    else if (argc == 1)
    {
        if (!is_value(argv[0]))
            error_expr(argv[0], "constant value expected.");
        out(REC_DATA, 0, 0, &op, 1);
        generate(argv[0], 2, false);
        out(REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    else
        error_expr(mnemonic, "invalid argument count.");
}

// Emits conditional jump instructions (Jcc).
// Emite instrucoes de salto condicional (Jcc).
static void emit_jump_cc(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    bool seg_offset = false;
    validate_distance(mnemonic, true, true, true);
    uint8_t op = opcode->op1;
    // Far jump with segment:offset as two separate arguments
    // Salto distante com segmento:offset como dois argumentos separados
    if (argc == 2 && mnemonic->force_far)
    {
        op = invert_comparsion(op);
        out(REC_DATA, 0, 0, &op, 1);
        op = 5;
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op3;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[0], -2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        seg_offset = generate(argv[1], -2, true);
        out(seg_offset ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Far jump with colon-separated or plain argument
    // Salto distante com argumento separado por dois pontos ou simples
    else if (argc == 1 && mnemonic->force_far)
    {
        if (argv[0]->token == TOK_COLON)
        {
            op = invert_comparsion(op);
            out(REC_DATA, 0, 0, &op, 1);
            op = 5;
            out(REC_DATA, 0, 0, &op, 1);
            op = opcode->op3;
            out(REC_DATA, 0, 0, &op, 1);
            out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
            seg_offset = generate(argv[0]->left, 2, true);
            out(seg_offset ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        }
        else
        {
            op = invert_comparsion(op);
            out(REC_DATA, 0, 0, &op, 1);
            op = 5;
            out(REC_DATA, 0, 0, &op, 1);
            op = opcode->op3;
            out(REC_DATA, 0, 0, &op, 1);
            out(generate(argv[0], 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
            seg_offset = generate(argv[0], 2, true);
            out(seg_offset ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        }
    }
    // Near jump with 16-bit relative offset
    // Salto proximo com offset relativo de 16 bits
    else if (argc == 1 && mnemonic->force_near)
    {
        op = invert_comparsion(op);
        out(REC_DATA, 0, 0, &op, 1);
        op = 3;
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op2;
        out(REC_DATA, 0, 0, &op, 1);
        if (generate(argv[0], -3, false))
        {
            out(REC_EXPR_PUSH_OFFSET, 2, 0, 0, 0);
            out(REC_EXPR_SUB, 0, 0, 0, 0);
            out(REC_EXPR_POP_INT16_RELOCATABLE_EMIT, 0, 0, 0, 0);
        }
        else
            out(REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Short jump with 8-bit relative offset
    // Salto curto com offset relativo de 8 bits
    else if (argc == 1 && !mnemonic->force_far && !mnemonic->force_near)
    {
        out(REC_DATA, 0, 0, &op, 1);
        if (generate(argv[0], -1, false))
        {
            out(REC_EXPR_PUSH_OFFSET, 1, 0, 0, 0);
            out(REC_EXPR_SUB, 0, 0, 0, 0);
        }
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
    }
    else
        error_expr(mnemonic, "invalid argument count.");
}

// Emits loop instructions (LOOP, LOOPZ, LOOPNZ, JCXZ, JECXZ).
// Emite instrucoes de loop (LOOP, LOOPZ, LOOPNZ, JCXZ, JECXZ).
static void emit_loop(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    bool seg_offset = false;
    validate_distance(mnemonic, true, true, true);
    uint8_t op = opcode->op1;
    // Far loop with segment:offset as two separate arguments
    // Loop distante com segmento:offset como dois argumentos separados
    if (argc == 2 && mnemonic->force_far)
    {
        out(REC_DATA, 0, 0, &op, 1);
        op = 2;
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op4;
        out(REC_DATA, 0, 0, &op, 1);
        op = 5;
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op3;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[0], 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        seg_offset = generate(argv[1], 2, true);
        out(seg_offset ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Far loop with colon-separated or plain argument
    // Loop distante com argumento separado por dois pontos ou simples
    else if (argc == 1 && mnemonic->force_far)
    {
        if (argv[0]->token == TOK_COLON)
        {
            out(REC_DATA, 0, 0, &op, 1);
            op = 2;
            out(REC_DATA, 0, 0, &op, 1);
            op = opcode->op4;
            out(REC_DATA, 0, 0, &op, 1);
            op = 5;
            out(REC_DATA, 0, 0, &op, 1);
            op = opcode->op3;
            out(REC_DATA, 0, 0, &op, 1);
            out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
            seg_offset = generate(argv[0]->left, 2, true);
            out(seg_offset ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        }
        else
        {
            out(REC_DATA, 0, 0, &op, 1);
            op = 2;
            out(REC_DATA, 0, 0, &op, 1);
            op = opcode->op4;
            out(REC_DATA, 0, 0, &op, 1);
            op = 5;
            out(REC_DATA, 0, 0, &op, 1);
            op = opcode->op3;
            out(REC_DATA, 0, 0, &op, 1);
            out(generate(argv[0], 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
            seg_offset = generate(argv[0], 2, true);
            out(seg_offset ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        }
    }
    // Near loop with 16-bit relative offset
    // Loop proximo com offset relativo de 16 bits
    else if (argc == 1 && mnemonic->force_near)
    {
        out(REC_DATA, 0, 0, &op, 1);
        op = 2;
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op4;
        out(REC_DATA, 0, 0, &op, 1);
        op = 3;
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op2;
        out(REC_DATA, 0, 0, &op, 1);
        if (generate(argv[0], -5, false))
        {
            out(REC_EXPR_PUSH_OFFSET, 2, 0, 0, 0);
            out(REC_EXPR_SUB, 0, 0, 0, 0);
            out(REC_EXPR_POP_INT16_RELOCATABLE_EMIT, 0, 0, 0, 0);
        }
        else
            out(REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Short loop with 8-bit relative offset
    // Loop curto com offset relativo de 8 bits
    else if (argc == 1 && !mnemonic->force_far && !mnemonic->force_near)
    {
        out(REC_DATA, 0, 0, &op, 1);
        if (generate(argv[0], -1, false))
        {
            out(REC_EXPR_PUSH_OFFSET, 1, 0, 0, 0);
            out(REC_EXPR_SUB, 0, 0, 0, 0);
        }
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
    }
    else
        error_expr(mnemonic, "invalid argument count.");
}

// Emits CALL or JMP instructions with near/far and register/memory addressing.
// Emite instrucoes CALL ou JMP com enderecamento proximo/distante e registrador/memoria.
static void emit_call(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    bool seg_offset = false;
    uint8_t op = opcode->op1;
    bool include_value = false;
    // op1: call near label offset
    // op2: call far label pointer
    // op3: call op4/5/6 first op
    // op4: call register pointer
    // op5: call near memory pointer
    // op6: call far memory pointer
    validate_distance(mnemonic, false, true, true);
    emit_seg_prefix(argc, argv);
    // Far call with segment:offset as two separate arguments
    // Call distante com segmento:offset como dois argumentos separados
    if (argc == 2 && mnemonic->force_far)
    {
        op = opcode->op2;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[0], 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        seg_offset = generate(argv[1], 2, true);
        out(seg_offset ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    // Far call with colon-separated or single plain argument
    // Call distante com argumento separado por dois pontos ou simples
    else if (argc == 1 && mnemonic->force_far && argv[0]->token != TOK_INDEX_OPEN)
    {
        if (argv[0]->token == TOK_COLON)
        {
            op = opcode->op2;
            out(REC_DATA, 0, 0, &op, 1);
            out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
            seg_offset = generate(argv[0]->left, 2, true);
            out(seg_offset ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        }
        else
        {
            op = opcode->op2;
            out(REC_DATA, 0, 0, &op, 1);
            out(generate(argv[0], 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
            seg_offset = generate(argv[0], 2, true);
            out(seg_offset ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        }
    }
    // Near call via 16-bit register
    // Call proximo via registrador de 16 bits
    else if (argc == 1 && is_reg_16bit(argv[0]))
    {
        op = opcode->op3;
        out(REC_DATA, 0, 0, &op, 1);
        op = opcode->op4 | 0b11000000 | argv[0]->reg->value;
        out(REC_DATA, 0, 0, &op, 1);
    }
    // Near/far call via direct memory address
    // Call proximo/distante via endereco de memoria direto
    else if (argc == 1 && is_address(argv[0]))
    {
        if (mnemonic->force_far)
        {
            op = opcode->op3;
            out(REC_DATA, 0, 0, &op, 1);
            op = opcode->op6 | 0b110;
            out(REC_DATA, 0, 0, &op, 1);
            out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        }
        else
        {
            op = opcode->op3;
            out(REC_DATA, 0, 0, &op, 1);
            op = opcode->op5 | 0b110;
            out(REC_DATA, 0, 0, &op, 1);
            out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        }
    }
    // Near/far call via register-based memory address
    // Call proximo/distante via endereco de memoria baseado em registrador
    else if (argc == 1 && is_reg_address(argv[0]))
    {
        if (mnemonic->force_far)
        {
            op = opcode->op3;
            out(REC_DATA, 0, 0, &op, 1);
            op = opcode->op6 | get_mrm(argv[0]);
            argv[0] = optimize(filter_registers(argv[0]));
            if (!(argv[0]->right->token == TOK_VALUE && argv[0]->right->value == 0 && (op & 0x7) != 0b110))
            {
                op |= 0b10000000;
                include_value = true;
            }
            out(REC_DATA, 0, 0, &op, 1);
            if (include_value)
                out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        }
        else
        {
            op = opcode->op3;
            out(REC_DATA, 0, 0, &op, 1);
            op = opcode->op5 | get_mrm(argv[0]);
            argv[0] = optimize(filter_registers(argv[0]));
            if (!(argv[0]->right->token == TOK_VALUE && argv[0]->right->value == 0 && (op & 0x7) != 0b110))
            {
                op |= 0b10000000;
                include_value = true;
            }
            out(REC_DATA, 0, 0, &op, 1);
            if (include_value)
                out(generate(argv[0]->right, 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        }
    }
    // Near call to relative offset (label)
    // Call proximo para offset relativo (rotulo)
    else if (argc == 1 && is_value(argv[0]))
    {
        op = opcode->op1;
        out(REC_DATA, 0, 0, &op, 1);
        if (generate(argv[0], -1, false))
        {
            out(REC_EXPR_PUSH_OFFSET, 2, 0, 0, 0);
            out(REC_EXPR_SUB, 0, 0, 0, 0);
            out(REC_EXPR_POP_INT16_RELOCATABLE_EMIT, 0, 0, 0, 0);
        }
        else
            out(REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    else
        error_expr(argv[0], "invalid arguments.");
}

reg_t _regs[] =
    {
        {"al", 0, 0, REG_8BIT},
        {"cl", 1, 1, REG_8BIT},
        {"dl", 2, 2, REG_8BIT},
        {"bl", 3, 3, REG_8BIT},
        {"ah", 4, 4, REG_8BIT},
        {"ch", 5, 5, REG_8BIT},
        {"dh", 6, 6, REG_8BIT},
        {"bh", 7, 7, REG_8BIT},
        {"ax", 0, 0, REG_16BIT},
        {"cx", 1, 1, REG_16BIT},
        {"dx", 2, 2, REG_16BIT},
        {"bx", 3, 3, REG_16BIT | REG_PTR},
        {"sp", 4, 4, REG_16BIT},
        {"bp", 5, 5, REG_16BIT | REG_PTR},
        {"si", 6, 6, REG_16BIT | REG_PTR},
        {"di", 7, 7, REG_16BIT | REG_PTR},
        {"es", 0, 0, REG_SEG},
        {"cs", 1, 1, REG_SEG},
        {"ss", 2, 2, REG_SEG},
        {"ds", 3, 3, REG_SEG},
        {NULL, 0, 0, 0}};

opcode_t _prefix[] =
    {
        {"es", 0b00100110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"cs", 0b00101110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"ss", 0b00110110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"ds", 0b00111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"lock", 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"rep", 0b11110011, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"repe", 0b11110011, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"repz", 0b11110011, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"repne", 0b11110010, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"repnz", 0b11110010, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {NULL, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, NULL}};

opcode_t _opcode[] =
    {
        {"aaa", 0b00110111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"aad", 0b11010101, 0b00001010, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple_2bytes},
        {"aam", 0b11010100, 0b00001010, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple_2bytes},
        {"aas", 0b00111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"adc", 0b00010000, 0b10000000, 0b00010000, 0b00010100, 0b00000000, 0b00000000, emit_mrm_simple},
        {"add", 0b00000000, 0b10000000, 0b00000000, 0b00000100, 0b00000000, 0b00000000, emit_mrm_simple},
        {"and", 0b00100000, 0b10000000, 0b00100000, 0b00100100, 0b00000000, 0b00000000, emit_mrm_simple},
        {"call", 0b11101000, 0b10011010, 0b11111111, 0b00010000, 0b00010000, 0b00011000, emit_call},
        {"cbw", 0b10011000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"clc", 0b11111000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"cld", 0b11111100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"cli", 0b11111010, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"cmc", 0b11110101, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"cmp", 0b00111000, 0b10000000, 0b00111000, 0b00111100, 0b00000000, 0b00000000, emit_mrm_simple},
        {"cmpsb", 0b10100110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"cmpsw", 0b10100111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"cwd", 0b10011001, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"daa", 0b00100111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"das", 0b00101111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"dec", 0b01001000, 0b11111110, 0b00001000, 0b00000000, 0b00000000, 0b00000000, emit_embbed_reg16bit_or_single_mrm},
        {"div", 0b11110110, 0b00110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_single_mrm},
        {"esc", 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_esc},
        {"hlt", 0b11110100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"idiv", 0b11110110, 0b00111000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_single_mrm},
        {"imul", 0b11110110, 0b00101000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_single_mrm},
        {"in", 0b11100100, 0b11101100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_input},
        {"inc", 0b01000000, 0b11111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_embbed_reg16bit_or_single_mrm},
        {"int", 0b11001100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_int},
        {"int3", 0b11001100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"into", 0b11001110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"iret", 0b11001111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"lahf", 0b10011111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"lds", 0b11000101, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_reg16bit_single_mrm},
        {"les", 0b11000100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_reg16bit_single_mrm},
        {"lea", 0b10001101, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_reg16bit_single_mrm},
        {"lodsb", 0b10101100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"lodsw", 0b10101101, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"mov", 0b10001000, 0b11000110, 0b00000000, 0b10100000, 0b10001100, 0b10110000, emit_mrm_complete},
        {"movsb", 0b10100100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"movsw", 0b10100101, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"mul", 0b11110110, 0b00100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_single_mrm},
        {"neg", 0b11110110, 0b00011000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_single_mrm},
        {"nop", 0b10010000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"not", 0b11110110, 0b00010000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_single_mrm},
        {"or", 0b00001000, 0b10000000, 0b00001000, 0b00001100, 0b00000000, 0b00000000, emit_mrm_simple},
        {"out", 0b11100110, 0b11101110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_output},
        {"outsb", 0b01101110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"outsw", 0b01101111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"pop", 0b01011000, 0b10001111, 0b00000000, 0b00000111, 0b00000000, 0b00000000, emit_push_pop},
        {"popa", 0b01100001, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"popf", 0b10011101, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"push", 0b01010000, 0b11111111, 0b00110000, 0b00000110, 0b00000000, 0b00000000, emit_push_pop},
        {"pusha", 0b01100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"pushf", 0b10011100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"rcl", 0b11010000, 0b00010000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_shift_rotate},
        {"rcr", 0b11010000, 0b00011000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_shift_rotate},
        {"ret", 0b11000010, 0b11001010, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_ret},
        {"retf", 0b11001010, 0b11001010, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_ret},
        {"rol", 0b11010000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_shift_rotate},
        {"ror", 0b11010000, 0b00001000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_shift_rotate},
        {"sahf", 0b10011110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"sal", 0b11010000, 0b00100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_shift_rotate},
        {"sar", 0b11010000, 0b00111000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_shift_rotate},
        {"shl", 0b11010000, 0b00100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_shift_rotate},
        {"shr", 0b11010000, 0b00101000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_shift_rotate},
        {"sbb", 0b00011000, 0b10000000, 0b00011000, 0b00011100, 0b00000000, 0b00000000, emit_mrm_simple},
        {"scasb", 0b10101110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"scasw", 0b10101111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"stc", 0b11111001, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"std", 0b11111101, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"sti", 0b11111011, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"stosb", 0b10101010, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"stosw", 0b10101011, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"sub", 0b00101000, 0b10000000, 0b00101000, 0b00101100, 0b00000000, 0b00000000, emit_mrm_simple},
        {"test", 0b10000100, 0b11110110, 0b00000000, 0b10101000, 0b00000000, 0b00000000, emit_mrm_simple},
        {"wait", 0b10011011, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"xchg", 0b10000110, 0b10000110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_mrm_simple},
        {"xlat", 0b11010111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, emit_simple},
        {"xor", 0b00110000, 0b10000000, 0b00110000, 0b00110100, 0b00000000, 0b00000000, emit_mrm_simple},
        {"jo", 0b01110000, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jno", 0b01110001, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jb", 0b01110010, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jnae", 0b01110010, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jc", 0b01110010, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jnb", 0b01110011, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jae", 0b01110011, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jnc", 0b01110011, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"je", 0b01110100, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jz", 0b01110100, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jne", 0b01110101, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jnz", 0b01110101, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jbe", 0b01110110, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jna", 0b01110110, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jnbe", 0b01110111, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"ja", 0b01110111, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"js", 0b01111000, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jns", 0b01111001, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jp", 0b01111010, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jpe", 0b01111010, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jnp", 0b01111011, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jpo", 0b01111011, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jl", 0b01111100, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jnge", 0b01111100, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jnl", 0b01111101, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jge", 0b01111101, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jle", 0b01111110, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jng", 0b01111110, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jnle", 0b01111111, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jg", 0b01111111, 0b11101001, 0b11101010, 0b00000000, 0b00000000, 0b00000000, emit_jump_cc},
        {"jcxz", 0b11100011, 0b11101001, 0b11101010, 0b11101011, 0b00000000, 0b00000000, emit_loop},
        {"jcxe", 0b11100011, 0b11101001, 0b11101010, 0b11101011, 0b00000000, 0b00000000, emit_loop},
        {"jecxz", 0b11100011, 0b11101001, 0b11101010, 0b11101011, 0b00000000, 0b00000000, emit_loop},
        {"jmp", 0b11101001, 0b11101010, 0b11111111, 0b00100000, 0b00100000, 0b00101000, emit_call},
        {"loop", 0b11100010, 0b11101001, 0b11101010, 0b11101011, 0b00000000, 0b00000000, emit_loop},
        {"loopz", 0b11100001, 0b11101001, 0b11101010, 0b11101011, 0b00000000, 0b00000000, emit_jump_cc},
        {"loope", 0b11100001, 0b11101001, 0b11101010, 0b11101011, 0b00000000, 0b00000000, emit_jump_cc},
        {"loopnz", 0b11100000, 0b11101001, 0b11101010, 0b11101011, 0b00000000, 0b00000000, emit_jump_cc},
        {"loopne", 0b11100000, 0b11101001, 0b11101010, 0b11101011, 0b00000000, 0b00000000, emit_jump_cc},

        {NULL, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, NULL}};
