#include "../asm.h"

rectype_t _cpu = REC_CPU_8085;

enum
{
    REG_8BIT = 0x01,
    REG_16BIT = 0x02,
    REG_16BIT_AUX = 0x4,
    REG_ONLY_B_D = 0x8,
    REG_16BIT_PSW = 0x10,
    REG_PTR = 0x20
};

static uint8_t gen_reg(expr_t *arg, int bits_offset, int group_filter, bool use_value_aux)
{
    uint8_t value = 0;
    if(arg->token == TOK_INDEX_OPEN && (group_filter & (REG_PTR | REG_ONLY_B_D)))
    {
        if(!arg->right || arg->right->token != TOK_REGISTER) error_expr(arg, "cpu register expected.");
        if((arg->right->reg->group & group_filter) == 0)
            error_expr(arg, "invalid register type [%x !& %x]", arg->reg->group, group_filter);
        value = arg->right->reg->value_aux & 0x7;
    }
    else
    {
        if(arg->token != TOK_REGISTER) error_expr(arg, "cpu register expected.");
        if((arg->reg->group & group_filter) == 0)
            error_expr(arg, "invalid register type [%x !& %x]", arg->reg->group, group_filter);
        value = use_value_aux ? (arg->reg->value_aux & 0x7) : (arg->reg->value & 0x7);
    }
    return value << bits_offset;
}

static void emit_simple(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    validate(mnemonic, false, false, false, false);
    if(argc != 0) error_expr(mnemonic, "invalid argument count.");
    out(REC_DATA, 0, 0, &opcode->op1, 1);
}

static void emit_8bit_value(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    validate(mnemonic, false, false, false, false);
    if(argc != 1) error_expr(mnemonic, "invalid argument count.");
    out(REC_DATA, 0, 0, &opcode->op1, 1);
    generate(argv[0], 1, false);
    out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
}

static void emit_8bit_value_with_mask_and_offset(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    validate(mnemonic, false, false, false, false);
    if(argc != 1) error_expr(mnemonic, "invalid argument count.");
    generate(argv[0], 0, false);
    out(REC_EXPR_PUSH_VALUE, opcode->op2, 0, 0, 0);
    out(REC_EXPR_AND, 0, 0, 0, 0);
    out(REC_EXPR_PUSH_VALUE, opcode->op3, 0, 0, 0);
    out(REC_EXPR_SHL, 0, 0, 0, 0);
    out(REC_EXPR_PUSH_VALUE, opcode->op1, 0, 0, 0);
    out(REC_EXPR_OR, 0, 0, 0, 0);
    out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
}

static void emit_16bit_value(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    validate(mnemonic, false, false, false, false);
    if(argc != 1) error_expr(mnemonic, "invalid argument count.");
    out(REC_DATA, 0, 0, &opcode->op1, 1);
    out(generate(argv[0], 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
}

static void emit_16bit_address(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    validate(mnemonic, false, false, false, false);
    if(argc != 1) error_expr(mnemonic, "invalid argument count.");
    out(REC_DATA, 0, 0, &opcode->op1, 1);
    out(generate(argv[0]->token == TOK_INDEX_OPEN ? argv[0]->right : argv[0], 2, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
}

static void emit_reg__reg___only8bit(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    validate(mnemonic, false, false, false, false);
    uint8_t op = opcode->op1;
    if(argc != 2) error_expr(mnemonic, "invalid argument count.");
    op |= gen_reg(argv[0], 3, REG_8BIT | REG_PTR, false);
    op |= gen_reg(argv[1], 0, REG_8BIT | REG_PTR, false);
    out(REC_DATA, 0, 0, &op, 1);
}

static void emit_reg__8bit_value___only8bit(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    validate(mnemonic, false, false, false, false);
    uint8_t op = opcode->op1;
    if(argc != 2) error_expr(mnemonic, "invalid argument count.");
    op |= gen_reg(argv[0], 3, REG_8BIT | REG_PTR, false);
    out(REC_DATA, 0, 0, &op, 1);
    generate(argv[1], 0, false);
    out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
}

static void emit_reg_with_offset___only8bit(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    validate(mnemonic, false, false, false, false);
    uint8_t op = opcode->op1;
    if(argc != 1) error_expr(mnemonic, "invalid argument count.");
    op |= gen_reg(argv[0], opcode->op2, REG_8BIT | REG_PTR, false);
    out(REC_DATA, 0, 0, &op, 1);
}

static void emit_reg_with_offset___only16bit(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    validate(mnemonic, false, false, false, false);
    uint8_t op = opcode->op1;
    if(argc != 1) error_expr(mnemonic, "invalid argument count.");
    op |= gen_reg(argv[0], opcode->op2, REG_16BIT, true);
    out(REC_DATA, 0, 0, &op, 1);
}

static void emit_reg_with_offset___only16bitaux(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    validate(mnemonic, false, false, false, false);
    uint8_t op = opcode->op1;
    if(argc != 1) error_expr(mnemonic, "invalid argument count.");
    op |= gen_reg(argv[0], opcode->op2, REG_16BIT_AUX, true);
    out(REC_DATA, 0, 0, &op, 1);
}

static void emit_reg_with_offset___only16bitpsw(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    validate(mnemonic, false, false, false, false);
    uint8_t op = opcode->op1;
    if(argc != 1) error_expr(mnemonic, "invalid argument count.");
    op |= gen_reg(argv[0], opcode->op2, REG_16BIT_PSW, true);
    out(REC_DATA, 0, 0, &op, 1);
}

static void emit_reg_with_offset___only_b_d(expr_t *mnemonic, opcode_t *opcode, int argc, expr_t *argv[])
{
    uint8_t op = opcode->op1;
    if(argc != 1) error_expr(mnemonic, "invalid argument count.");
    op |= gen_reg(argv[0], opcode->op2, REG_ONLY_B_D, true);
    out(REC_DATA, 0, 0, &op, 1);
}

reg_t _regs[] = 
{
    {"b", 0, 0, REG_8BIT | REG_16BIT | REG_16BIT_AUX | REG_ONLY_B_D | REG_16BIT_PSW},
    {"c", 1, 0, REG_8BIT},
    {"d", 2, 1, REG_8BIT | REG_16BIT | REG_16BIT_AUX | REG_ONLY_B_D | REG_16BIT_PSW},
    {"e", 3, 0, REG_8BIT},
    {"h", 4, 2, REG_8BIT | REG_16BIT | REG_16BIT_AUX | REG_16BIT_PSW},
    {"l", 5, 0, REG_8BIT},
    {"m", 6, 3, REG_8BIT | REG_16BIT_AUX | REG_PTR},
    {"a", 7, 0, REG_8BIT},

    {"bc", 0, 0, REG_16BIT | REG_16BIT_AUX | REG_16BIT_PSW | REG_ONLY_B_D},
    {"de", 1, 1, REG_16BIT | REG_16BIT_AUX | REG_16BIT_PSW | REG_ONLY_B_D},
    {"hl", 2, 2, REG_16BIT | REG_16BIT_AUX | REG_16BIT_PSW | REG_PTR},
    {"sp", 3, 3, REG_16BIT},

    //{"b", 0, 0, REG_16BIT_AUX},
    //{"d", 1, 1, REG_16BIT_AUX},
    //{"h", 2, 2, REG_16BIT_AUX},
    //{"m", 3, 3, REG_16BIT_AUX},

    //{"b", 0, 0, REG_16BIT_PSW},
    //{"d", 1, 1, REG_16BIT_PSW},
    //{"h", 2, 2, REG_16BIT_PSW},
    {"psw", 3, 3, REG_16BIT_PSW},

    {NULL, 0, 0, 0}
};

opcode_t _prefix[] =
{
    {NULL,      0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, NULL}
};

opcode_t _opcode[] =
{
    {"nop",     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"mov",     0x40, 0x00, 0x00, 0x00, 0x00, 0x00, emit_reg__reg___only8bit},
    {"add",     0x80, 0x00, 0x00, 0x00, 0x00, 0x00, emit_reg_with_offset___only8bit},
    {"adc",     0x88, 0x00, 0x00, 0x00, 0x00, 0x00, emit_reg_with_offset___only8bit},
    {"sub",     0x90, 0x00, 0x00, 0x00, 0x00, 0x00, emit_reg_with_offset___only8bit},
    {"sbb",     0x98, 0x00, 0x00, 0x00, 0x00, 0x00, emit_reg_with_offset___only8bit},
    {"ana",     0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, emit_reg_with_offset___only8bit},
    {"xra",     0xa8, 0x00, 0x00, 0x00, 0x00, 0x00, emit_reg_with_offset___only8bit},
    {"ora",     0xb0, 0x00, 0x00, 0x00, 0x00, 0x00, emit_reg_with_offset___only8bit},
    {"cmp",     0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, emit_reg_with_offset___only8bit},
    {"lxi",     0x01, 0x04, 0x00, 0x00, 0x00, 0x00, emit_reg_with_offset___only16bit},
    {"stax",    0x02, 0x04, 0x00, 0x00, 0x00, 0x00, emit_reg_with_offset___only_b_d},
    {"inx",     0x03, 0x04, 0x00, 0x00, 0x00, 0x00, emit_reg_with_offset___only16bit},
    {"dcx",     0x0b, 0x04, 0x00, 0x00, 0x00, 0x00, emit_reg_with_offset___only16bit},
    {"inr",     0x04, 0x03, 0x00, 0x00, 0x00, 0x00, emit_reg_with_offset___only8bit},
    {"dcr",     0x05, 0x03, 0x00, 0x00, 0x00, 0x00, emit_reg_with_offset___only8bit},
    {"rlc",     0x07, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"ral",     0x17, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"daa",     0x27, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"stc",     0x37, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"rrc",     0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"rar",     0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"cma",     0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"cmc",     0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"shld",    0x22, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_address},
    {"sta",     0x32, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_address},
    {"lhld",    0x2a, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_address},
    {"lda",     0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_address},
    {"mvi",     0x06, 0x00, 0x00, 0x00, 0x00, 0x00, emit_reg__8bit_value___only8bit},
    {"rnz",     0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"rnc",     0xd0, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"rpo",     0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"rp",      0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"pop",     0xc1, 0x04, 0x00, 0x00, 0x00, 0x00, emit_reg_with_offset___only16bitpsw},
    {"push",    0xc5, 0x04, 0x00, 0x00, 0x00, 0x00, emit_reg_with_offset___only16bitpsw},
    {"jnz",     0xc2, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_value},
    {"jnc",     0xd2, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_value},
    {"jpo",     0xe2, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_value},
    {"jp",      0xf2, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_value},
    {"jmp",     0xc3, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_value},
    {"out",     0xd3, 0x00, 0x00, 0x00, 0x00, 0x00, emit_8bit_value},
    {"xthl",    0xe3, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"di",      0xf3, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"cnz",     0xc4, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_value},
    {"cnc",     0xd4, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_value},
    {"cpo",     0xe4, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_value},
    {"cp",      0xf4, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_value},
    {"adi",     0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, emit_8bit_value},
    {"sui",     0xd6, 0x00, 0x00, 0x00, 0x00, 0x00, emit_8bit_value},
    {"ani",     0xe6, 0x00, 0x00, 0x00, 0x00, 0x00, emit_8bit_value},
    {"ori",     0xf6, 0x00, 0x00, 0x00, 0x00, 0x00, emit_8bit_value},
    {"rz",      0xc8, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"rc",      0xd8, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"rpe",     0xe8, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"rm",      0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"ret",     0xc9, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"pchl",    0xe9, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"sphl",    0xf9, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"jz",      0xca, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_value},
    {"jc",      0xda, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_value},
    {"jpe",     0xea, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_value},
    {"jm",      0xfa, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_value},
    {"in",      0xdb, 0x00, 0x00, 0x00, 0x00, 0x00, emit_8bit_value},
    {"xchg",    0xeb, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"ei",      0xfb, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"cz",      0xcc, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_value},
    {"cc",      0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_value},
    {"cpe",     0xec, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_value},
    {"cm",      0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_value},
    {"call",    0xcd, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_value},
    {"aci",     0xce, 0x00, 0x00, 0x00, 0x00, 0x00, emit_8bit_value},
    {"sbi",     0xde, 0x00, 0x00, 0x00, 0x00, 0x00, emit_8bit_value},
    {"xri",     0xee, 0x00, 0x00, 0x00, 0x00, 0x00, emit_8bit_value},
    {"cpi",     0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, emit_8bit_value},
    {"rst",     0xc7, 0x07, 0x03, 0x00, 0x00, 0x00, emit_8bit_value_with_mask_and_offset},
    {"hlt",     0x76, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"arhl",    0x10, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"rim",     0x20, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"sim",     0x30, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"dsub",    0x08, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"rdel",    0x18, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"ldhi",    0x28, 0x00, 0x00, 0x00, 0x00, 0x00, emit_8bit_value},
    {"ldsi",    0x38, 0x00, 0x00, 0x00, 0x00, 0x00, emit_8bit_value},
    {"shlx",    0xd9, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"jnk",     0xdd, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_value},
    {"lhlx",    0xed, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {"jk",      0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, emit_16bit_value},
    {"rstv",    0xcb, 0x00, 0x00, 0x00, 0x00, 0x00, emit_simple},
    {NULL,      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, NULL}
};