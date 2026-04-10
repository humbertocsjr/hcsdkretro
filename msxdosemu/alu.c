#include "emu.h"

uint8_t alu_inc_byte(uint8_t value)
{
    hf_set((value & 0xf) == 0xf);
    pvf_set(value == 0x7f);
    value++;
    sf_set(value & 128);
    zf_set(value == 0);
    nf_set(0);
    return value;
}

uint8_t alu_dec_byte(uint8_t value)
{
    pvf_set(value == 0x80);
    value--;
    hf_set((value & 0xf) == 0xf);
    sf_set(value & 128);
    zf_set(value == 0);
    nf_set(1);
    return value;
}

uint8_t alu_rlca(uint8_t value)
{
    cf_set(value & 128);
    nf_set(0);
    hf_set(0);
    value = (value << 1) | ((value >> 7) & 1);
    return value;
}

uint8_t alu_rlc(uint8_t value)
{
    cf_set(value & 128);
    nf_set(0);
    hf_set(0);
    value = (value << 1) | ((value >> 7) & 1);
    alu_parity(value);
    zf_set(value == 0);
    sf_set(value & 128);
    return value;
}

uint8_t alu_rrca(uint8_t value)
{
    cf_set(value & 1);
    nf_set(0);
    hf_set(0);
    value = (value >> 1) | ((value << 7) & 128);
    return value;
}

uint8_t alu_rrc(uint8_t value)
{
    cf_set(value & 1);
    nf_set(0);
    hf_set(0);
    value = (value >> 1) | ((value << 7) & 128);
    alu_parity(value);
    zf_set(value == 0);
    sf_set(value & 128);
    return value;
}

uint8_t alu_sla(uint8_t value)
{
    cf_set(value & 128);
    nf_set(0);
    hf_set(0);
    value = (value << 1);
    zf_set(value == 0);
    sf_set(value & 128);
    alu_parity(value);
    return value;
}

uint8_t alu_sra(uint8_t value)
{
    cf_set(value & 1);
    nf_set(0);
    hf_set(0);
    value = (value >> 1) | (value & 128);
    zf_set(value == 0);
    sf_set(value & 128);
    alu_parity(value);
    return value;
}

uint8_t alu_sll(uint8_t value)
{
    cf_set(value & 128);
    nf_set(0);
    hf_set(0);
    value = (value << 1) | 1;
    zf_set(value == 0);
    sf_set(value & 128);
    alu_parity(value);
    return value;
}

uint8_t alu_srl(uint8_t value)
{
    cf_set(value & 1);
    nf_set(0);
    hf_set(0);
    value = (value >> 1);
    zf_set(value == 0);
    sf_set(value & 128);
    alu_parity(value);
    return value;
}

uint8_t alu_rla(uint8_t value)
{
    uint8_t bit = cf_get() ? 1 : 0;
    nf_set(0);
    hf_set(0);
    cf_set(value & 128);
    value = (value << 1) | bit;
    //zf_set(value == 0);
    alu_parity(value);
    return value;
}

uint8_t alu_rra(uint8_t value)
{
    uint8_t bit = cf_get() ? 128 : 0;
    nf_set(0);
    hf_set(0);
    cf_set(value & 1);
    value = (value >> 1) | bit;
    //zf_set(value == 0);
    alu_parity(value);
    return value;
}

uint8_t alu_rl(uint8_t value)
{
    uint8_t bit = cf_get() ? 1 : 0;
    nf_set(0);
    hf_set(0);
    cf_set(value & 128);
    value = (value << 1) | bit;
    zf_set(value == 0);
    sf_set(value & 128);
    alu_parity(value);
    return value;
}

uint8_t alu_rr(uint8_t value)
{
    uint8_t bit = cf_get() ? 128 : 0;
    nf_set(0);
    hf_set(0);
    cf_set(value & 1);
    value = (value >> 1) | bit;
    zf_set(value == 0);
    sf_set(value & 128);
    alu_parity(value);
    return value;
}

uint16_t alu_add_word(uint16_t value1, uint16_t value2)
{
    uint16_t tmp = value1;
    uint32_t u = (uint32_t)value1 + (uint32_t)value2;
    hf_set(((value1 & 0xf) + (value2 & 0xf)) >> 4);
    value1 += value2;
    cf_set(u & 0xff0000);
    nf_set(0);
    return value1;
}

uint8_t alu_add_byte(uint8_t value1, uint8_t value2)
{
    int8_t s1 = (*(int8_t*)&value1);
    int8_t s2 = (*(int8_t*)&value2);
    int16_t s = s1 + s2;
    uint16_t u = (uint16_t)value1 + (uint16_t)value2;
    uint8_t tmp = value1;
    hf_set(((value1 & 0xf) + (value2 & 0xf)) >> 4);
    value1 += value2;
    cf_set(u & 0xff00);
    nf_set(0);
    zf_set(value1 == 0);
    sf_set(value1 & 128);
    pvf_set(s > 127 || s < -128);
    return value1;
}

uint8_t alu_adc_byte(uint8_t value1, uint8_t value2)
{
    int8_t s1 = (*(int8_t*)&value1);
    int8_t s2 = (*(int8_t*)&value2);
    int16_t s = (int16_t)s1 + (int16_t)s2 + (int16_t)(cf_get() ? 1 : 0);
    uint16_t u = (uint16_t)value1 + (uint16_t)value2 + (cf_get() ? 1 : 0);
    uint8_t tmp = value1;
    hf_set(((value1 & 0xf) + (value2 & 0xf) + (cf_get() ? 1 : 0)) >> 4);
    value1 += value2 + (cf_get() ? 1 : 0);
    //hf_set((tmp ^ value2 ^ value1) & 0x10);
    cf_set(u & 0xff00);
    nf_set(0);
    zf_set(value1 == 0);
    sf_set(value1 & 128);
    pvf_set(s > 127 || s < -128 ); 
    return value1;
}

uint16_t alu_adc_word(uint16_t value1, uint16_t value2)
{
    int16_t s1 = (*(int16_t*)&value1);
    int16_t s2 = (*(int16_t*)&value2);
    int32_t s = s1 + s2 + (cf_get() ? 1 : 0);
    uint32_t u = (uint32_t)value1 + (uint32_t)value2 + (cf_get() ? 1 : 0);
    uint16_t tmp = value1;
    hf_set(((value1 & 0xf) + (value2 & 0xf) + (cf_get() ? 1 : 0)) >> 4);
    value1 += value2 + (cf_get() ? 1 : 0);
    cf_set(u & 0xff0000);
    nf_set(0);
    zf_set(value1 == 0);
    sf_set(value1 & 0x8000);
    pvf_set(s > 32767 || s < -32768);
    return value1;
}

uint8_t alu_sub_byte(uint8_t value1, uint8_t value2)
{
    int8_t s1 = (*(int8_t*)&value1);
    int8_t s2 = (*(int8_t*)&value2);
    int16_t s = s1 - s2;
    uint16_t u = (uint16_t)value1 - (uint16_t)value2;
    uint8_t tmp = value1;
    hf_set(((value1 & 0xf) - (value2 & 0xf)) >> 4);
    value1 -= value2;
    cf_set(u & 0xff00);
    nf_set(1);
    zf_set(value1 == 0);
    sf_set(value1 & 128);
    pvf_set(s > 127 || s < -128 ); // || tmp < value2
    return value1;
}

uint8_t alu_cp_byte(uint8_t value1, uint8_t value2)
{
    int8_t s1 = (*(int8_t*)&value1);
    int8_t s2 = (*(int8_t*)&value2);
    int16_t s = s1 - s2;
    uint16_t u = (uint16_t)value1 - (uint16_t)value2;
    uint8_t tmp = value1;
    hf_set(((value1 & 0xf) < (value2 & 0xf)));
    value1 -= value2;
    cf_set(u & 0xff00);
    nf_set(1);
    zf_set(value1 == 0);
    sf_set(value1 & 128);
    pvf_set(s > 127 || s < -128 );
    return value1;
}

uint8_t alu_neg_byte(uint8_t value)
{
    int8_t s = -(*(int8_t*)&value);
    uint16_t u = (*(uint8_t*)&s);
    pvf_set(value == 0x80);
    cf_set(value != 0x00);
    value = u;
    hf_set((value & 0xf) != 0);
    nf_set(1);
    zf_set(value == 0);
    sf_set(value & 128);
    return value;
}

uint8_t alu_sbc_byte(uint8_t value1, uint8_t value2)
{
    int8_t s1 = (*(int8_t*)&value1);
    int8_t s2 = (*(int8_t*)&value2);
    int16_t s = s1 - s2;
    uint16_t u = (uint16_t)value1 - (uint16_t)(value2 + (cf_get() ? 1 : 0));
    uint8_t tmp = value1;
    hf_set(((value1 & 0xf) < ((value2 + (cf_get() ? 1 : 0)) & 0xf)));
    value1 -= value2 + (cf_get() ? 1 : 0);
    cf_set(u & 0xff00);
    nf_set(1);
    zf_set(value1 == 0);
    sf_set(value1 & 128);
    pvf_set(s > 127 || s < -128 );
    return value1;
}

uint16_t alu_sbc_word(uint16_t value1, uint16_t value2)
{
    int16_t s1 = (*(int16_t*)&value1);
    int16_t s2 = (*(int16_t*)&value2);
    int32_t s = s1 - (s2 + (cf_get() ? 1 : 0));
    uint16_t tmp = value1;
    hf_set(((value1) - ((value2 + (cf_get() ? 1 : 0)))) & 0xf00);
    value1 -= value2 + (cf_get() ? 1 : 0);
    cf_set(tmp < (value2 + (cf_get() ? 1 : 0)));
    nf_set(1);
    zf_set(value1 == 0);
    sf_set(value1 & 0x8000);
    pvf_set(s > 32767 || s < -32768);
    return value1;
}

uint8_t alu_and_byte(uint8_t value1, uint8_t value2)
{
    cf_set(0);
    nf_set(0);
    hf_set(1);
    value1 &= value2;
    alu_parity(value1);
    zf_set(value1 == 0);
    sf_set(value1 & 128);
    return value1;
}

void alu_bit(uint8_t value, uint8_t bit)
{
    hf_set(1);
    nf_set(0);
    zf_set((value & (1 << bit)) == 0);
    sf_set((value & (1 << bit)) != 0 && bit == 7);
    //pvf_set(value & (1 << bit));
    alu_parity(value & (1 << bit));
}

uint8_t alu_xor_byte(uint8_t value1, uint8_t value2)
{
    cf_set(0);
    nf_set(0);
    hf_set(0);
    value1 ^= value2;
    alu_parity(value1);
    zf_set(value1 == 0);
    sf_set(value1 & 128);
    return value1;
}

uint8_t alu_or_byte(uint8_t value1, uint8_t value2)
{
    cf_set(0);
    nf_set(0);
    hf_set(0);
    value1 |= value2;
    alu_parity(value1);
    zf_set(value1 == 0);
    sf_set(value1 & 128);
    return value1;
}

void alu_parity(uint8_t value)
{
    int bits = 0;
    for (size_t i = 0; i < 8; i++)
    {
        bits += ((value >> i) & 1) ? 1 : 0;
    }
    pvf_set((bits & 1) == 0);
}

uint8_t alu_daa(uint8_t value)
{
    uint8_t old_value = value;
    uint16_t u = value;
    if(nf_get())
    {
        if(cf_get())
        {
            value -= 0x60;
            cf_set(1);
        }
        else cf_set(0);
        if(hf_get())
        {
            value -= 0x6;
            hf_set(0);
        }
        else hf_set(0);
    }
    else
    {
        if(((value >> 4) & 0xf) > 9 || cf_get())
        {
            value += 0x60;
            cf_set(1);
        }
        else cf_set(0);
        if((value & 0xf) > 9 || hf_get())
        {
            value += 0x6;
            hf_set(1);
        }
        else hf_set(0);
        if(((value >> 4) & 0xf) > 9)
        {
            value += 0x60;
            cf_set(1);
        }
    }
    zf_set(value == 0);
    sf_set(value & 128);
    alu_parity(value);
    return value;
}