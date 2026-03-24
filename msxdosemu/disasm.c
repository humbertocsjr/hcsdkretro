#include "emu.h"

char _disasm[256];
char _disasm_arg0[32];
char _disasm_arg1[32];

static void fmt(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(_disasm, 255, fmt, args);
    va_end(args);
}

static void fmt_arg(int arg, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(arg == 0 ?_disasm_arg0 : _disasm_arg1, 31, fmt, args);
    va_end(args);
}

enum pre_e
{
    PRE_NO,
    PRE_CB,
    PRE_ED,
    PRE_DD,
    PRE_DDCB,
    PRE_FD,
    PRE_FDCB
};

char *disasm_reg8(uint8_t opcode, int offset, enum pre_e prefix, bool *has_index)
{
    switch((opcode >> offset) & 3)
    {
        case 0: return "b";
        case 1: return "c";
        case 2: return "d";
        case 3: return "e";
        case 4: return prefix == PRE_DD ? "ixh" : prefix == PRE_FD ? "iyh" : "h";
        case 5: return prefix == PRE_DD ? "ixl" : prefix == PRE_FD ? "iyl" : "l";
        case 6: *has_index = true; return (prefix == PRE_DD || prefix == PRE_DDCB) ? "[ix+%i]]" : (prefix == PRE_FD || prefix == PRE_FDCB) ? "[iy+%i]" :"[hl]";
        case 7: return "a";
    }
    return "";
}

char *disasm_reg16(uint8_t opcode, int offset, enum pre_e prefix, char *last)
{
    switch((opcode >> offset) & 3)
    {
        case 0: return "bc";
        case 1: return "de";
        case 2: return prefix == PRE_DD ? "ix" : prefix == PRE_FD ? "iy" : "hl";
        case 3: return last;
    }
    return "";
}

uint16_t disasm_16(void *ptr)
{
    return *(uint16_t*)ptr;
}

void disasm(uint16_t address)
{
    uint8_t *ptr = &_memory[address];
    enum pre_e prefix = PRE_NO;
    bool has_index = false;
    int8_t offset = 0;
    strcpy(_disasm, "UNKNOWN");
    if(*ptr == 0xdd)
    {
        prefix = PRE_DD;
        ptr++;
    }
    else if(*ptr == 0xfd)
    {
        prefix = PRE_FD;
        ptr++;
    }
    else if(*ptr == 0xed)
    {
        prefix = PRE_ED;
        ptr++;
    }
    if(*ptr == 0xcb && prefix != PRE_ED)
    {
        prefix = prefix == PRE_DD ? PRE_DDCB : prefix == PRE_FD ? PRE_FDCB : PRE_CB;
        ptr++;
        if(prefix == PRE_DDCB || prefix == PRE_FDCB)
        {
            offset = *(int8_t*)ptr;
            ptr++;
        }
    }
    uint8_t op = *ptr++;
    switch (prefix)
    {
        case PRE_NO:
        case PRE_DD:
        case PRE_FD:
            switch(op & 0xc0)
            {
                case 0x00:
                    switch(op & 0x07)
                    {
                        case 0x00:
                            switch(op & 0x38)
                            {
                                case 0x00: fmt("nop"); break;
                                case 0x08: fmt("ex af, af'"); break;
                                case 0x10: fmt("djnz %i", *ptr++); break;
                                case 0x18: fmt("jr %i", *ptr++); break;
                                case 0x20: fmt("jr nz, %i", *ptr++); break;
                                case 0x28: fmt("jr z, %i", *ptr++); break;
                                case 0x30: fmt("jr nc, %i", *ptr++); break;
                                case 0x38: fmt("jr c, %i", *ptr++); break;
                            }
                            break;
                        case 0x01:
                            switch(op & 0x08)
                            {
                                case 0x00: fmt("ld %s, 0x%04x", disasm_reg16(op, 4, prefix, "sp"), disasm_16(ptr)); break;
                                case 0x08: fmt("add hl, %s", disasm_reg16(op, 4, prefix, "sp")); break;
                            }
                            break;
                        case 0x02:
                            switch(op & 0x38)
                            {
                                case 0x00:
                                case 0x10: fmt("ld [%s], a", disasm_reg16(op, 4, prefix, "sp")); break;
                                case 0x08: 
                                case 0x18: fmt("ld a, [%s]", disasm_reg16(op, 4, prefix, "sp")); break;
                                case 0x20: fmt("ld [%i], %s", disasm_16(ptr), disasm_reg16(2, 0, prefix, "")); break;
                                case 0x28: fmt("ld %s, [%i]", disasm_reg16(2, 0, prefix, ""), disasm_16(ptr)); break;
                                case 0x30: fmt("ld [%i], a", disasm_16(ptr)); break;
                                case 0x38: fmt("ld a, [%i]", disasm_16(ptr)); break;
                            }
                            break;
                        case 0x03:
                            switch(op & 0x08)
                            {
                                case 0x00: fmt("inc %s", disasm_reg16(op, 4, prefix, "sp")); break;
                                case 0x08: fmt("dec %s", disasm_reg16(op, 4, prefix, "sp")); break;
                            }
                            break;
                        case 0x04:
                            fmt_arg(0, disasm_reg8(op, 3, prefix, &has_index), offset);
                            fmt("inc %s", _disasm_arg0);
                            break;
                        case 0x05:
                            fmt_arg(0, disasm_reg8(op, 3, prefix, &has_index), offset);
                            fmt("dec %s", _disasm_arg0);
                            break;
                        case 0x06:
                            fmt_arg(0, disasm_reg8(op, 3, prefix, &has_index), offset);
                            fmt("ld %s, 0x%02x ; '%c'", _disasm_arg0, (unsigned)*ptr, printable(*ptr));
                            break;
                        case 0x07:
                            switch(op & 0x38)
                            {
                                case 0x00: fmt("rlca"); break;
                                case 0x08: fmt("rrca"); break;
                                case 0x10: fmt("rla"); break;
                                case 0x18: fmt("rra"); break;
                                case 0x20: fmt("daa"); break;
                                case 0x28: fmt("cpl"); break;
                                case 0x30: fmt("scf"); break;
                                case 0x38: fmt("ccf"); break;
                            }
                            break;
                    }
                    break;
                case 0x40:
                    fmt_arg(0, disasm_reg8(op, 3, prefix, &has_index), offset);
                    fmt_arg(1, disasm_reg8(op, 0, prefix, &has_index), offset);
                    fmt("ld %s, %s", _disasm_arg0, _disasm_arg1);
                    break;
                case 0x80:
                    fmt_arg(0, disasm_reg8(op, 0, prefix, &has_index), offset);
                    switch (op & 0x38)
                    {
                        case 0x00: fmt("add a, %s", _disasm_arg0); break;
                        case 0x08: fmt("adc a, %s", _disasm_arg0); break;
                        case 0x10: fmt("sub %s", _disasm_arg0); break;
                        case 0x18: fmt("sbc a, %s", _disasm_arg0); break;
                        case 0x20: fmt("and %s", _disasm_arg0); break;
                        case 0x28: fmt("xor %s", _disasm_arg0); break;
                        case 0x30: fmt("or %s", _disasm_arg0); break;
                        case 0x38: fmt("cp %s", _disasm_arg0); break;
                    }
                    break;
                case 0xc0:
                    switch(op & 0x07)
                    {
                        case 0x00:
                            switch(op & 0x38)
                            {
                                case 0x00: fmt("ret nz"); break;
                                case 0x08: fmt("ret z"); break;
                                case 0x10: fmt("ret nc"); break;
                                case 0x18: fmt("ret c"); break;
                                case 0x20: fmt("ret po"); break;
                                case 0x28: fmt("ret pe"); break;
                                case 0x30: fmt("ret p"); break;
                                case 0x38: fmt("ret m"); break;
                            }
                            break;
                        case 0x01:
                            switch(op & 0x38)
                            {
                                case 0x00:
                                case 0x10:
                                case 0x20:
                                case 0x30: fmt("pop %s", disasm_reg16(op, 4, prefix, "af")); break;
                                case 0x08: fmt("ret"); break;
                                case 0x18: fmt("exx"); break;
                                case 0x28: fmt("jp [hl]"); break;
                                case 0x38: fmt("ld sp, hl"); break;
                            }
                            break;
                        case 0x02:
                            switch(op & 0x38)
                            {
                                case 0x00: fmt("jp nz, %i", disasm_16(ptr)); break;
                                case 0x08: fmt("jp z, %i", disasm_16(ptr)); break;
                                case 0x10: fmt("jp nc, %i", disasm_16(ptr)); break;
                                case 0x18: fmt("jp c, %i", disasm_16(ptr)); break;
                                case 0x20: fmt("jp po, %i", disasm_16(ptr)); break;
                                case 0x28: fmt("jp pe, %i", disasm_16(ptr)); break;
                                case 0x30: fmt("jp p, %i", disasm_16(ptr)); break;
                                case 0x38: fmt("jp m, %i", disasm_16(ptr)); break;
                            }
                            break;
                        case 0x03:
                            switch(op & 0x38)
                            {
                                case 0x00: fmt("jp %i", disasm_16(ptr)); break;
                                case 0x10: fmt("out [%i], a", *ptr); break;
                                case 0x18: fmt("in a, [%i]", *ptr); break;
                                case 0x20: fmt("ex [sp], hl"); break;
                                case 0x28: fmt("ex de, hl"); break;
                                case 0x30: fmt("di"); break;
                                case 0x38: fmt("ei"); break;
                            }
                            break;
                        case 0x04:
                            switch(op & 0x38)
                            {
                                case 0x00: fmt("call nz, %i", disasm_16(ptr)); break;
                                case 0x08: fmt("call z, %i", disasm_16(ptr)); break;
                                case 0x10: fmt("call nc, %i", disasm_16(ptr)); break;
                                case 0x18: fmt("call c, %i", disasm_16(ptr)); break;
                                case 0x20: fmt("call po, %i", disasm_16(ptr)); break;
                                case 0x28: fmt("call pe, %i", disasm_16(ptr)); break;
                                case 0x30: fmt("call p, %i", disasm_16(ptr)); break;
                                case 0x38: fmt("call m, %i", disasm_16(ptr)); break;
                            }
                            break;
                        case 0x05:
                            switch(op & 0x38)
                            {
                                case 0x00:
                                case 0x10:
                                case 0x20:
                                case 0x30: fmt("push %s", disasm_reg16(op, 4, prefix, "af")); break;
                                case 0x08: fmt("call 0x%04x", disasm_16(ptr)); break;
                            }
                            break;
                        case 0x06:
                            switch(op & 0x38)
                            {
                                case 0x00: fmt("add a, 0x%02x", *ptr); break;
                                case 0x08: fmt("adc a, 0x%02x", *ptr); break;
                                case 0x10: fmt("sub 0x%02x", *ptr); break;
                                case 0x18: fmt("sbc a, 0x%02x", *ptr); break;
                                case 0x20: fmt("and 0x%02x", *ptr); break;
                                case 0x28: fmt("xor 0x%02x", *ptr); break;
                                case 0x30: fmt("or 0x%02x", *ptr); break;
                                case 0x38: fmt("cp 0x%02x", *ptr); break;
                            }
                            break;
                        case 0x07:
                            switch(op & 0x38)
                            {
                                case 0x00: fmt("rst 0x00"); break;
                                case 0x08: fmt("rst 0x08"); break;
                                case 0x10: fmt("rst 0x10"); break;
                                case 0x18: fmt("rst 0x18"); break;
                                case 0x20: fmt("rst 0x20"); break;
                                case 0x28: fmt("rst 0x28"); break;
                                case 0x30: fmt("rst 0x30"); break;
                                case 0x38: fmt("rst 0x38"); break;
                            }
                            break;
                    }
                    break;
            }
            break;
        case PRE_ED:
            break;
        case PRE_CB:
        case PRE_DDCB:
        case PRE_FDCB:
            break;
    }
}