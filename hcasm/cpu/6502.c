#include "../asm.h"

rectype_t _cpu = REC_CPU_6502;

enum
{
    REG_8BIT = 0x01,
    REG_ACC = 0x02,
    REG_IDX_X = 0x04,
    REG_IDX_Y = 0x08
};

// --== Register Table ==--

reg_t _regs[] =
    {
        {"a", 0, 0, REG_8BIT | REG_ACC},
        {"x", 1, 0, REG_8BIT | REG_IDX_X},
        {"y", 2, 0, REG_8BIT | REG_IDX_Y},
        {NULL, 0, 0, 0}};

// --== Prefixes (none) ==--

opcode_t _prefix[] =
    {
        {NULL, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, NULL}};

// --== Helper functions ==--

static bool is_acc(expr_t *e)
{
    return e->token == TOK_REGISTER && (e->reg->group & REG_ACC);
}

static bool is_x(expr_t *e)
{
    return e->token == TOK_REGISTER && (e->reg->group & REG_IDX_X);
}

static bool is_y(expr_t *e)
{
    return e->token == TOK_REGISTER && (e->reg->group & REG_IDX_Y);
}

static bool is_imm(expr_t *e)
{
    return e->immediate;
}

static bool is_value_only(expr_t *e)
{
    if (!e)
        return false;
    switch (e->token)
    {
    case TOK_VALUE:
        return true;
    case TOK_SYMBOL:
        return true;
    case TOK_CURRENT_POS:
        return true;
    case TOK_REGISTER:
        return false;
    case TOK_INDEX_OPEN:
        return false;
    default:
        if (e->left && !is_value_only(e->left))
            return false;
        if (e->right && !is_value_only(e->right))
            return false;
        return true;
    }
}

static bool is_zp_value(int value)
{
    return value >= 0 && value < 0x100;
}

// Addressing mode enum (must match alu_opc table column order)
enum
{
    AM_IMM,  // 0
    AM_ZP,   // 1
    AM_ZPX,  // 2
    AM_ABS,  // 3
    AM_ABSX, // 4
    AM_ABSY, // 5
    AM_INDX, // 6
    AM_INDY, // 7
    AM_ZPY,  // 8 (not in alu_opc)
    AM_ACC,  // 9
    AM_REL,  // 10
    AM_IND   // 11
};

static int detect_alu_mode(int argc, expr_t *argv[])
{
    if (argc == 1 && is_imm(argv[0]))
        return AM_IMM;
    if (argc == 1)
    {
        if (argv[0]->token == TOK_VALUE)
        {
            if (is_zp_value(argv[0]->value))
                return AM_ZP;
            return AM_ABS;
        }
        if (argv[0]->token == TOK_SYMBOL || argv[0]->token == TOK_SUB_LABEL ||
            argv[0]->token == TOK_ADD  || argv[0]->token == TOK_SUB ||
            argv[0]->token == TOK_LOBYTE || argv[0]->token == TOK_HIBYTE)
            return AM_ABS;
    }
    if (argc == 2 && is_x(argv[1]))
    {
        if (argv[0]->token == TOK_VALUE && is_zp_value(argv[0]->value))
            return AM_ZPX;
        return AM_ABSX;
    }
    if (argc == 2 && argv[0]->token == TOK_INDEX_OPEN && is_y(argv[1]))
        return AM_INDY;
    if (argc == 2 && is_y(argv[1]))
        return AM_ABSY;
    if (argc == 1 && argv[0]->token == TOK_INDEX_OPEN)
    {
        expr_t *inner = argv[0]->right;
        if (inner->token == TOK_ADD &&
            ((is_x(inner->left) && is_value_only(inner->right)) ||
             (is_x(inner->right) && is_value_only(inner->left))))
            return AM_INDX;
    }
    return -1;
}

// --== Opcode tables ==--

// IMM, ZP, ZPX, ABS, ABSX, ABSY, INDX, INDY
static const uint8_t alu_opc[8][8] =
    {
        {0x69, 0x65, 0x75, 0x6D, 0x7D, 0x79, 0x61, 0x71}, // 0=ADC
        {0x29, 0x25, 0x35, 0x2D, 0x3D, 0x39, 0x21, 0x31}, // 1=AND
        {0xC9, 0xC5, 0xD5, 0xCD, 0xDD, 0xD9, 0xC1, 0xD1}, // 2=CMP
        {0x49, 0x45, 0x55, 0x4D, 0x5D, 0x59, 0x41, 0x51}, // 3=EOR
        {0xA9, 0xA5, 0xB5, 0xAD, 0xBD, 0xB9, 0xA1, 0xB1}, // 4=LDA
        {0x09, 0x05, 0x15, 0x0D, 0x1D, 0x19, 0x01, 0x11}, // 5=ORA
        {0xE9, 0xE5, 0xF5, 0xED, 0xFD, 0xF9, 0xE1, 0xF1}, // 6=SBC
        {0x00, 0x85, 0x95, 0x8D, 0x9D, 0x99, 0x81, 0x91}, // 7=STA
};

// 3 high bits for RMW modes: ZP, ZPX, ABS, ABSX
static const uint8_t rmw_base[6][4] =
    {
        {0x06, 0x16, 0x0E, 0x1E}, // 0=ASL (ACC: 0x0A)
        {0x46, 0x56, 0x4E, 0x5E}, // 1=LSR (ACC: 0x4A)
        {0x26, 0x36, 0x2E, 0x3E}, // 2=ROL (ACC: 0x2A)
        {0x66, 0x76, 0x6E, 0x7E}, // 3=ROR (ACC: 0x6A)
        {0xE6, 0xF6, 0xEE, 0xFE}, // 4=INC (no ACC)
        {0xC6, 0xD6, 0xCE, 0xDE}, // 5=DEC (no ACC)
};

static const uint8_t rmw_acc[] = {0x0A, 0x4A, 0x2A, 0x6A};

// LDX/LDY/STX/STY
// IMM, ZP, ZPX(ABSX), ZPY, ABS, ABSY(ABSY)
// Inst order: 0=LDX, 1=LDY, 2=STX, 3=STY
static const uint8_t idx_opc[4][6] =
    {
        {0xA2, 0xA6, 0x00, 0xB6, 0xAE, 0xBE}, // LDX: IMM,ZP,--,ZPY,ABS,ABSY
        {0xA0, 0xA4, 0xB4, 0x00, 0xAC, 0xBC}, // LDY: IMM,ZP,ZPX,--,ABS,ABSX
        {0x00, 0x86, 0x00, 0x96, 0x8E, 0x00}, // STX: --,ZP,--,ZPY,ABS,--
        {0x00, 0x84, 0x94, 0x00, 0x8C, 0x00}, // STY: --,ZP,ZPX,--,ABS,--
};

// CPX/CPY: IMM=base, ZP=base+4, ABS=base+0x0C
static const uint8_t cpxy_base[] = {0xE0, 0xC0};

// Branch opcodes
static const uint8_t branch_opc[] =
    {
        0x90, 0xB0, 0xF0, 0x30, 0xD0, 0x10, 0x50, 0x70};

// Jump opcodes
#define JMP_ABS 0x4C
#define JMP_IND 0x6C
#define JSR_ABS 0x20

// --== Simple (implied/accumulator) emitter ==--

static void emit_simple(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    if (argc != 0)
        error_expr(mnemonic, "invalid argument count.");
    out(REC_DATA, 0, 0, &opcode->op1, 1);
}

// --== Branch emitter ==--

static void emit_branch(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op;
    if (argc != 1)
        error_expr(mnemonic, "invalid argument count.");
    op = opcode->op1;
    out(REC_DATA, 0, 0, &op, 1);
    if (generate(argv[0], 0, false))
    {
        out(REC_EXPR_PUSH_OFFSET, 1, 0, 0, 0);
        out(REC_EXPR_SUB, 0, 0, 0, 0);
    }
    out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
}

// --== ALU / Load / Store emitter (ADC, AND, CMP, EOR, LDA, ORA, SBC, STA) ==--

static void emit_alu(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    int inst = opcode->op1;
    int mode = detect_alu_mode(argc, argv);
    if (mode == -1)
        error_expr(mnemonic, "invalid addressing mode.");
    if (mode == AM_IMM && inst == 7)
        error_expr(mnemonic, "immediate mode not available for STA.");

    uint8_t op = alu_opc[inst][mode];
    out(REC_DATA, 0, 0, &op, 1);

    switch (mode)
    {
    case AM_IMM:
        generate(argv[0], 1, false);
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
        break;
    case AM_ZP:
    case AM_INDX:
        if (argv[0]->token == TOK_INDEX_OPEN)
            generate(argv[0]->right, 1, false);
        else
            generate(argv[0], 1, false);
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
        break;
    case AM_ZPX:
    case AM_ZPY:
        generate(argv[0], 1, false);
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
        break;
    case AM_ABS:
        out(generate(argv[0], 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        break;
    case AM_ABSX:
    case AM_ABSY:
        out(generate(argv[0], 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        break;
    case AM_INDY:
        generate(argv[0]->right, 1, false);
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
        break;
    default:
        error_expr(mnemonic, "invalid addressing mode.");
    }
}

// --== Read-Modify-Write emitter (ASL, LSR, ROL, ROR, INC, DEC) ==--

static void emit_rmw(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    int inst = opcode->op1;
    uint8_t op;

    if (argc == 1 && is_acc(argv[0]))
    {
        if (inst > 3)
            error_expr(mnemonic, "accumulator mode not available.");
        op = rmw_acc[inst];
        out(REC_DATA, 0, 0, &op, 1);
        return;
    }

    int mode;
    if (argc == 1 && argv[0]->token == TOK_VALUE)
    {
        if (is_zp_value(argv[0]->value))
            mode = 0;
        else
            mode = 2;
    }
    else if (argc == 2 && is_x(argv[1]))
    {
        if (argv[0]->token == TOK_VALUE && is_zp_value(argv[0]->value))
            mode = 1;
        else
            mode = 3;
    }
    else if (argc == 1 && argv[0]->token == TOK_INDEX_OPEN)
    {
        if (is_x(argv[0]->right) || is_value_only(argv[0]->right))
            mode = 1;
        else
            error_expr(mnemonic, "invalid addressing mode.");
    }
    else
        error_expr(mnemonic, "invalid addressing mode.");

    op = rmw_base[inst][mode];
    out(REC_DATA, 0, 0, &op, 1);

    if (mode == 0 || mode == 1)
    {
        generate(argv[0], 1, false);
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
    }
    else
    {
        out(generate(argv[0], 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
}

// --== Index register emitter (LDX, LDY, STX, STY) ==--

static void emit_idx(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    int inst = opcode->op1;
    uint8_t op;
    int mode = -1;

    // Detect mode specific to LDX/LDY/STX/STY
    if (argc == 1 && is_imm(argv[0]) && (inst == 0 || inst == 1))
    {
        op = idx_opc[inst][0];
        out(REC_DATA, 0, 0, &op, 1);
        generate(argv[0], 1, false);
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
        return;
    }

    if (argc == 1 && argv[0]->token == TOK_VALUE)
    {
        if (is_zp_value(argv[0]->value))
            op = idx_opc[inst][1];
        else
            op = idx_opc[inst][4];
        out(REC_DATA, 0, 0, &op, 1);
        if (is_zp_value(argv[0]->value))
        {
            out(REC_DATA, 0, 0, &argv[0]->value, 1);
        }
        else
        {
            out(generate(argv[0], 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        }
        return;
    }

    if (argc == 2)
    {
        if (is_y(argv[1]) && (inst == 0 || inst == 2))
        {
            if (is_zp_value(argv[0]->value))
                op = idx_opc[inst][3];
            else if (inst == 0)
                op = idx_opc[inst][5];
            else
                error_expr(mnemonic, "invalid addressing mode.");
        }
        else if (is_x(argv[1]) && (inst == 1 || inst == 3))
        {
            if (is_zp_value(argv[0]->value))
                op = idx_opc[inst][2];
            else if (inst == 1)
                op = idx_opc[inst][5];
            else
                error_expr(mnemonic, "invalid addressing mode.");
        }
        else
            error_expr(mnemonic, "invalid addressing mode.");

        if (op == 0)
            error_expr(mnemonic, "invalid addressing mode for this instruction.");

        out(REC_DATA, 0, 0, &op, 1);
        if (is_zp_value(argv[0]->value))
        {
            out(REC_DATA, 0, 0, &argv[0]->value, 1);
        }
        else
        {
            out(generate(argv[0], 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
        }
        return;
    }

    error_expr(mnemonic, "invalid argument count.");
}

// --== Compare X/Y emitter (CPX, CPY) ==--

static void emit_cpxy(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t base = opcode->op1;
    uint8_t op;

    if (argc == 1 && is_imm(argv[0]))
    {
        op = base;
        out(REC_DATA, 0, 0, &op, 1);
        generate(argv[0], 1, false);
        out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
    }
    else if (argc == 1 && argv[0]->token == TOK_VALUE && is_zp_value(argv[0]->value))
    {
        op = base + 4;
        out(REC_DATA, 0, 0, &op, 1);
        out(REC_DATA, 0, 0, &argv[0]->value, 1);
    }
    else if (argc == 1)
    {
        op = base + 0x0C;
        out(REC_DATA, 0, 0, &op, 1);
        out(generate(argv[0], 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    else
        error_expr(mnemonic, "invalid argument count.");
}

// --== Jump/Subroutine emitter (JMP, JSR) ==--

static void emit_jmp(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    if (argc != 1)
        error_expr(mnemonic, "invalid argument count.");
    if (argv[0]->token == TOK_INDEX_OPEN)
    {
        out(REC_DATA, 0, 0, &opcode->op2, 1);
        out(generate(argv[0]->right, 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
    else
    {
        out(REC_DATA, 0, 0, &opcode->op1, 1);
        out(generate(argv[0], 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
    }
}

// --== Opcode table ==--

opcode_t _opcode[] =
    {
        {"adc", 0, 0, 0, 0, 0, 0, emit_alu},
        {"and", 1, 0, 0, 0, 0, 0, emit_alu},
        {"asl", 0, 0, 0, 0, 0, 0, emit_rmw},
        {"bcc", 0x90, 0, 0, 0, 0, 0, emit_branch},
        {"bcs", 0xB0, 0, 0, 0, 0, 0, emit_branch},
        {"beq", 0xF0, 0, 0, 0, 0, 0, emit_branch},
        {"bit", 0x24, 0x2C, 0, 0, 0, 0, emit_alu},
        {"bmi", 0x30, 0, 0, 0, 0, 0, emit_branch},
        {"bne", 0xD0, 0, 0, 0, 0, 0, emit_branch},
        {"bpl", 0x10, 0, 0, 0, 0, 0, emit_branch},
        {"brk", 0x00, 0, 0, 0, 0, 0, emit_simple},
        {"bvc", 0x50, 0, 0, 0, 0, 0, emit_branch},
        {"bvs", 0x70, 0, 0, 0, 0, 0, emit_branch},
        {"clc", 0x18, 0, 0, 0, 0, 0, emit_simple},
        {"cld", 0xD8, 0, 0, 0, 0, 0, emit_simple},
        {"cli", 0x58, 0, 0, 0, 0, 0, emit_simple},
        {"clv", 0xB8, 0, 0, 0, 0, 0, emit_simple},
        {"cmp", 2, 0, 0, 0, 0, 0, emit_alu},
        {"cpx", 0xE0, 0, 0, 0, 0, 0, emit_cpxy},
        {"cpy", 0xC0, 0, 0, 0, 0, 0, emit_cpxy},
        {"dec", 5, 0, 0, 0, 0, 0, emit_rmw},
        {"dex", 0xCA, 0, 0, 0, 0, 0, emit_simple},
        {"dey", 0x88, 0, 0, 0, 0, 0, emit_simple},
        {"eor", 3, 0, 0, 0, 0, 0, emit_alu},
        {"inc", 4, 0, 0, 0, 0, 0, emit_rmw},
        {"inx", 0xE8, 0, 0, 0, 0, 0, emit_simple},
        {"iny", 0xC8, 0, 0, 0, 0, 0, emit_simple},
        {"jmp", JMP_ABS, JMP_IND, 0, 0, 0, 0, emit_jmp},
        {"jsr", JSR_ABS, 0, 0, 0, 0, 0, emit_jmp},
        {"lda", 4, 0, 0, 0, 0, 0, emit_alu},
        {"ldx", 0, 0, 0, 0, 0, 0, emit_idx},
        {"ldy", 1, 0, 0, 0, 0, 0, emit_idx},
        {"lsr", 1, 0, 0, 0, 0, 0, emit_rmw},
        {"nop", 0xEA, 0, 0, 0, 0, 0, emit_simple},
        {"ora", 5, 0, 0, 0, 0, 0, emit_alu},
        {"pha", 0x48, 0, 0, 0, 0, 0, emit_simple},
        {"php", 0x08, 0, 0, 0, 0, 0, emit_simple},
        {"pla", 0x68, 0, 0, 0, 0, 0, emit_simple},
        {"plp", 0x28, 0, 0, 0, 0, 0, emit_simple},
        {"rol", 2, 0, 0, 0, 0, 0, emit_rmw},
        {"ror", 3, 0, 0, 0, 0, 0, emit_rmw},
        {"rti", 0x40, 0, 0, 0, 0, 0, emit_simple},
        {"rts", 0x60, 0, 0, 0, 0, 0, emit_simple},
        {"sbc", 6, 0, 0, 0, 0, 0, emit_alu},
        {"sec", 0x38, 0, 0, 0, 0, 0, emit_simple},
        {"sed", 0xF8, 0, 0, 0, 0, 0, emit_simple},
        {"sei", 0x78, 0, 0, 0, 0, 0, emit_simple},
        {"sta", 7, 0, 0, 0, 0, 0, emit_alu},
        {"stx", 2, 0, 0, 0, 0, 0, emit_idx},
        {"sty", 3, 0, 0, 0, 0, 0, emit_idx},
        {"tax", 0xAA, 0, 0, 0, 0, 0, emit_simple},
        {"tay", 0xA8, 0, 0, 0, 0, 0, emit_simple},
        {"tsx", 0xBA, 0, 0, 0, 0, 0, emit_simple},
        {"txa", 0x8A, 0, 0, 0, 0, 0, emit_simple},
        {"txs", 0x9A, 0, 0, 0, 0, 0, emit_simple},
        {"tya", 0x98, 0, 0, 0, 0, 0, emit_simple},

        {NULL, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, NULL}};
