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
    switch((opcode >> offset) & 7)
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

char *disasm_mainreg(enum pre_e prefix)
{
    switch(prefix)
    {
        case PRE_DD: return "ix";
        case PRE_FD: return "iy";
        default: return "hl";
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
                                case 0x10: fmt("djnz 0x%04x", *(int8_t*)ptr++ + address + 2); break;
                                case 0x18: fmt("jr 0x%04x", *(int8_t*)ptr++ + address + 2); break;
                                case 0x20: fmt("jr nz, 0x%04x", *(int8_t*)ptr++ + address + 2); break;
                                case 0x28: fmt("jr z, 0x%04x", *(int8_t*)ptr++ + address + 2); break;
                                case 0x30: fmt("jr nc, 0x%04x", *(int8_t*)ptr++ + address + 2); break;
                                case 0x38: fmt("jr c, 0x%04x", *(int8_t*)ptr++ + address + 2); break;
                            }
                            break;
                        case 0x01:
                            switch(op & 0x08)
                            {
                                case 0x00: fmt("ld %s, 0x%04x", disasm_reg16(op, 4, prefix, "sp"), disasm_16(ptr)); break;
                                case 0x08: fmt("add %s, %s", disasm_mainreg(prefix), disasm_reg16(op, 4, prefix, "sp")); break;
                            }
                            break;
                        case 0x02:
                            if(prefix == PRE_DD || prefix == PRE_FD)
                            {
                                offset = *(int8_t*)ptr;
                                ptr++;
                            }
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
                            if(prefix == PRE_DD || prefix == PRE_FD)
                            {
                                offset = *(int8_t*)ptr;
                                ptr++;
                            }
                            fmt_arg(0, disasm_reg8(op, 3, prefix, &has_index), offset);
                            fmt("inc %s", _disasm_arg0);
                            break;
                        case 0x05:
                            if(prefix == PRE_DD || prefix == PRE_FD)
                            {
                                offset = *(int8_t*)ptr;
                                ptr++;
                            }
                            fmt_arg(0, disasm_reg8(op, 3, prefix, &has_index), offset);
                            fmt("dec %s", _disasm_arg0);
                            break;
                        case 0x06:
                            if(prefix == PRE_DD || prefix == PRE_FD)
                            {
                                offset = *(int8_t*)ptr;
                                ptr++;
                            }
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
                    if(prefix == PRE_DD || prefix == PRE_FD)
                    {
                        offset = *(int8_t*)ptr;
                        ptr++;
                    }
                    fmt_arg(0, disasm_reg8(op, 3, (op == 0x66 || op == 0x6e) ? PRE_NO : prefix, &has_index), offset);
                    fmt_arg(1, disasm_reg8(op, 0, (op == 0x74 || op == 0x75) ? PRE_NO : prefix, &has_index), offset);
                    fmt("ld %s, %s", _disasm_arg0, _disasm_arg1);
                    break;
                case 0x80:
                    if(prefix == PRE_DD || prefix == PRE_FD)
                    {
                        offset = *(int8_t*)ptr;
                        ptr++;
                    }
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
                                case 0x28: fmt("jp [%s]", disasm_mainreg(prefix)); break;
                                case 0x38: fmt("ld sp, %s", disasm_mainreg(prefix)); break;
                            }
                            break;
                        case 0x02:
                            switch(op & 0x38)
                            {
                                case 0x00: fmt("jp nz, 0x%04x", disasm_16(ptr)); break;
                                case 0x08: fmt("jp z, 0x%04x", disasm_16(ptr)); break;
                                case 0x10: fmt("jp nc, 0x%04x", disasm_16(ptr)); break;
                                case 0x18: fmt("jp c, 0x%04x", disasm_16(ptr)); break;
                                case 0x20: fmt("jp po, 0x%04x", disasm_16(ptr)); break;
                                case 0x28: fmt("jp pe, 0x%04x", disasm_16(ptr)); break;
                                case 0x30: fmt("jp p, 0x%04x", disasm_16(ptr)); break;
                                case 0x38: fmt("jp m, 0x%04x", disasm_16(ptr)); break;
                            }
                            break;
                        case 0x03:
                            switch(op & 0x38)
                            {
                                case 0x00: fmt("jp 0x%04x", disasm_16(ptr)); break;
                                case 0x10: fmt("out [%i], a", *ptr); break;
                                case 0x18: fmt("in a, [%i]", *ptr); break;
                                case 0x20: fmt("ex [sp], %s", disasm_mainreg(prefix)); break;
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
            switch (op)
            {
                case 0x40: fmt("in b, [c]"); break;
                case 0x50: fmt("in d, [c]"); break;
                case 0x60: fmt("in h, [c]"); break;
                case 0x70: fmt("in [c]"); break;
                case 0x41: fmt("out [c], b"); break;
                case 0x51: fmt("out [c], d"); break;
                case 0x61: fmt("out [c], h"); break;
                case 0x71: fmt("out [c], 0"); break;
                case 0x42: fmt("sbc hl, bc"); break;
                case 0x52: fmt("sbc hl, de"); break;
                case 0x62: fmt("sbc hl, hl"); break;
                case 0x72: fmt("sbc hl, sp"); break;
                case 0x43: fmt("ld [0x%04x], bc", disasm_16(ptr)); break;
                case 0x53: fmt("ld [0x%04x], de", disasm_16(ptr)); break;
                case 0x63: fmt("ld [0x%04x], hl", disasm_16(ptr)); break;
                case 0x73: fmt("ld [0x%04x], sp", disasm_16(ptr)); break;
                case 0x44: fmt("neg"); break;
                case 0x45: fmt("retn"); break;
                case 0x46: fmt("im 0"); break;
                case 0x56: fmt("im 1"); break;
                case 0x47: fmt("ld i, a"); break;
                case 0x57: fmt("ld a, i"); break;
                case 0x67: fmt("rrd"); break;
                case 0x48: fmt("in c, [c]"); break;
                case 0x58: fmt("in e, [c]"); break;
                case 0x68: fmt("in l, [c]"); break;
                case 0x78: fmt("in a, [c]"); break;
                case 0x49: fmt("out [c], c"); break;
                case 0x59: fmt("out [c], e"); break;
                case 0x69: fmt("out [c], l"); break;
                case 0x79: fmt("out [c], a"); break;
                case 0x4a: fmt("adc hl, bc"); break;
                case 0x5a: fmt("adc hl, de"); break;
                case 0x6a: fmt("adc hl, hl"); break;
                case 0x7a: fmt("adc hl, sp"); break;
                case 0x4b: fmt("ld bc, [0x%04x]", disasm_16(ptr)); break;
                case 0x5b: fmt("ld de, [0x%04x]", disasm_16(ptr)); break;
                case 0x6b: fmt("ld hl, [0x%04x]", disasm_16(ptr)); break;
                case 0x7b: fmt("ld sp, [0x%04x]", disasm_16(ptr)); break;
                case 0x4d: fmt("reti"); break;
                case 0x5e: fmt("im 2"); break;
                case 0x4f: fmt("ld r, a"); break;
                case 0x5f: fmt("ld a, r"); break;
                case 0x6f: fmt("rld"); break;
                case 0xa0: fmt("ldi"); break;
                case 0xb0: fmt("ldir"); break;
                case 0xa1: fmt("cpi"); break;
                case 0xb1: fmt("cpir"); break;
                case 0xa2: fmt("ini"); break;
                case 0xb2: fmt("inir"); break;
                case 0xa3: fmt("outi"); break;
                case 0xb3: fmt("otir"); break;
                case 0xa8: fmt("ldd"); break;
                case 0xb8: fmt("lddr"); break;
                case 0xa9: fmt("cpd"); break;
                case 0xb9: fmt("cpdr"); break;
                case 0xaa: fmt("ind"); break;
                case 0xba: fmt("indr"); break;
                case 0xab: fmt("outd"); break;
                case 0xbb: fmt("otdr"); break;
            }
            break;
        case PRE_CB:
        case PRE_DDCB:
        case PRE_FDCB:
            // TODO: Create switch case
            if(prefix == PRE_DDCB) fmt("0xDD 0xCB 0x%2XX", op);
            else if(prefix == PRE_FDCB) fmt("0xFD 0xCB 0x%2XX", op);
            else fmt("0xCB 0x%2XX", op);
            break;
    }
}