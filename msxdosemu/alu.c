#include "emu.h"

// [English] Increment byte with flag updates
// [Portuguese] Incrementa byte com atualização de flags
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

// [English] Decrement byte with flag updates
// [Portuguese] Decrementa byte com atualização de flags
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

// [English] Rotate left circular accumulator (RLCA)
// [Portuguese] Rotaciona à esquerda circular no acumulador
uint8_t alu_rlca(uint8_t value)
{
    cf_set(value & 128);
    nf_set(0);
    hf_set(0);
    value = (value << 1) | ((value >> 7) & 1);
    xf_set(value & (1 << 3));
    yf_set(value & (1 << 5));
    return value;
}

// [English] Rotate left circular (RLC)
// [Portuguese] Rotaciona à esquerda circular
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

// [English] Rotate right circular accumulator (RRCA)
// [Portuguese] Rotaciona à direita circular no acumulador
uint8_t alu_rrca(uint8_t value)
{
    cf_set(value & 1);
    nf_set(0);
    hf_set(0);
    value = (value >> 1) | ((value << 7) & 128);
    xf_set(value & (1 << 3));
    yf_set(value & (1 << 5));
    return value;
}

// [English] Rotate right circular (RRC)
// [Portuguese] Rotaciona à direita circular
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

// [English] Shift left arithmetic (SLA)
// [Portuguese] Deslocamento aritmético à esquerda
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

// [English] Shift right arithmetic (SRA)
// [Portuguese] Deslocamento aritmético à direita
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

// [English] Shift left logical (SLL)
// [Portuguese] Deslocamento lógico à esquerda com inserção de 1
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

// [English] Shift right logical (SRL)
// [Portuguese] Deslocamento lógico à direita
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

// [English] Rotate left accumulator through carry (RLA)
// [Portuguese] Rotaciona à esquerda via carry no acumulador
uint8_t alu_rla(uint8_t value)
{
    uint8_t bit = cf_get() ? 1 : 0;
    nf_set(0);
    hf_set(0);
    cf_set(value & 128);
    value = (value << 1) | bit;
    xf_set(value & (1 << 3));
    yf_set(value & (1 << 5));
    return value;
}

// [English] Rotate right accumulator through carry (RRA)
// [Portuguese] Rotaciona à direita via carry no acumulador
uint8_t alu_rra(uint8_t value)
{
    uint8_t bit = cf_get() ? 128 : 0;
    nf_set(0);
    hf_set(0);
    cf_set(value & 1);
    value = (value >> 1) | bit;
    xf_set(value & (1 << 3));
    yf_set(value & (1 << 5));
    return value;
}

// [English] Rotate left through carry (RL)
// [Portuguese] Rotaciona à esquerda via carry
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

// [English] Rotate right through carry (RR)
// [Portuguese] Rotaciona à direita via carry
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

// [English] Add 16-bit word (ADD HL, rr)
// [Portuguese] Soma word de 16 bits
uint16_t alu_add_word(uint16_t value1, uint16_t value2)
{
    uint32_t u = (uint32_t)value1 + (uint32_t)value2;
    hf_set(((value1 & 0xf) + (value2 & 0xf)) >> 4);
    value1 += value2;
    cf_set(u & 0xff0000);
    nf_set(0);
    return value1;
}

// [English] Add byte with full flag updates
// [Portuguese] Soma byte com atualização completa de flags
uint8_t alu_add_byte(uint8_t value1, uint8_t value2)
{
    uint8_t _v1 = value1;
    int8_t s1 = (*(int8_t*)&value1);
    int8_t s2 = (*(int8_t*)&value2);
    int16_t s = s1 + s2;
    uint16_t u = (uint16_t)value1 + (uint16_t)value2;
    hf_set(((value1 & 0xf) + (value2 & 0xf)) >> 4);
    value1 += value2;
    cf_set(u & 0xff00);
    nf_set(0);
    zf_set(value1 == 0);
    sf_set(value1 & 128);
    pvf_set(s > 127 || s < -128);
    xf_set(value1 & (1 << 3));
    yf_set(value1 & (1 << 5));
    return value1;
}

// [English] Add byte with carry (ADC)
// [Portuguese] Soma byte com carry
uint8_t alu_adc_byte(uint8_t value1, uint8_t value2)
{
    uint8_t _v1 = value1;
    int8_t s1 = (*(int8_t*)&value1);
    int8_t s2 = (*(int8_t*)&value2);
    uint8_t _carry = cf_get() ? 1 : 0;
    int16_t s = (int16_t)s1 + (int16_t)s2 + (int16_t)_carry;
    uint16_t u = (uint16_t)value1 + (uint16_t)value2 + _carry;
    hf_set(((value1 & 0xf) + (value2 & 0xf) + _carry) >> 4);
    value1 += value2 + _carry;
    cf_set(u & 0xff00);
    nf_set(0);
    zf_set(value1 == 0);
    sf_set(value1 & 128);
    pvf_set(s > 127 || s < -128 );
    xf_set(value1 & (1 << 3));
    yf_set(value1 & (1 << 5));
    return value1;
}

// [English] Add 16-bit word with carry (ADC HL, rr)
// [Portuguese] Soma word 16 bits com carry
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

// [English] Subtract byte with flag updates
// [Portuguese] Subtrai byte com atualização de flags
uint8_t alu_sub_byte(uint8_t value1, uint8_t value2)
{
    uint8_t _v1 = value1;
    int8_t s1 = (*(int8_t*)&value1);
    int8_t s2 = (*(int8_t*)&value2);
    int16_t s = s1 - s2;
    uint16_t u = (uint16_t)value1 - (uint16_t)value2;
    hf_set(((value1 & 0xf) < (value2 & 0xf)));
    value1 -= value2;
    cf_set(u & 0xff00);
    nf_set(1);
    zf_set(value1 == 0);
    sf_set(value1 & 128);
    pvf_set(s > 127 || s < -128 );
    xf_set(value1 & (1 << 3));
    yf_set(value1 & (1 << 5));
    return value1;
}

// [English] Compare byte (CP) - sets flags only, no result stored
// [Portuguese] Compara byte - só altera flags, sem armazenar resultado
uint8_t alu_cp_byte(uint8_t value1, uint8_t value2)
{
    int8_t s1 = (*(int8_t*)&value1);
    int8_t s2 = (*(int8_t*)&value2);
    int16_t s = s1 - s2;
    uint16_t u = (uint16_t)value1 - (uint16_t)value2;
    uint8_t tmp = value1;
    hf_set(((value1 & 0xf) < (value2 & 0xf)));
    value1 -= value2;
    xf_set(value2 & (1 << 3));
    yf_set(value2 & (1 << 5));
    cf_set(u & 0xff00);
    nf_set(1);
    zf_set(value1 == 0);
    sf_set(value1 & 128);
    pvf_set(s > 127 || s < -128 );
    return value1;
}

// [English] Extended compare byte (for CPI/CPD)
// [Portuguese] Comparação estendida de byte (para CPI/CPD)
uint8_t alu_cp_extended_byte(uint8_t value1, uint8_t value2)
{
    int8_t s1 = (*(int8_t*)&value1);
    int8_t s2 = (*(int8_t*)&value2);
    int16_t s = s1 - s2;
    uint16_t u = (uint16_t)value1 - (uint16_t)value2;
    uint8_t tmp = value1;
    hf_set(((value1 & 0xf) < (value2 & 0xf)));
    value1 -= value2;
    xf_set((value1 - (hf_get() ? 1 : 0)) & (1 << 3));
    yf_set((value1 - (hf_get() ? 1 : 0)) & (1 << 1));
    nf_set(1);
    zf_set(value1 == 0);
    sf_set(value1 & 128);
    pvf_set(s > 127 || s < -128 );
    return value1;
}

// [English] Negate accumulator (NEG)
// [Portuguese] Nega o acumulador
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
    xf_set(value & (1 << 3));
    yf_set(value & (1 << 5));
    return value;
}

// [English] Subtract byte with carry (SBC)
// [Portuguese] Subtrai byte com carry
uint8_t alu_sbc_byte(uint8_t value1, uint8_t value2)
{
    uint8_t _v1 = value1;
    int8_t s1 = (*(int8_t*)&value1);
    int8_t s2 = (*(int8_t*)&value2);
    uint8_t _carry = cf_get() ? 1 : 0;
    int16_t s = (int16_t)s1 - (int16_t)s2 - (int16_t)_carry;
    uint16_t u = (uint16_t)value1 - (uint16_t)(value2 + _carry);
    hf_set(((value1 & 0xf) < ((value2 + _carry) & 0xf)));
    value1 -= value2 + _carry;
    cf_set(u & 0xff00);
    nf_set(1);
    zf_set(value1 == 0);
    sf_set(value1 & 128);
    pvf_set(s > 127 || s < -128 );
    xf_set(value1 & (1 << 3));
    yf_set(value1 & (1 << 5));
    return value1;
}

// [English] Subtract 16-bit word with carry (SBC HL, rr)
// [Portuguese] Subtrai word 16 bits com carry
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

// [English] Logical AND byte
// [Portuguese] AND lógico entre bytes
uint8_t alu_and_byte(uint8_t value1, uint8_t value2)
{
    cf_set(0);
    nf_set(0);
    hf_set(1);
    value1 &= value2;
    alu_parity(value1);
    zf_set(value1 == 0);
    sf_set(value1 & 128);
    xf_set(value1 & (1 << 3));
    yf_set(value1 & (1 << 5));
    return value1;
}

// [English] Test bit (BIT)
// [Portuguese] Testa um bit do valor
void alu_bit(uint8_t value, uint8_t bit)
{
    hf_set(1);
    nf_set(0);
    zf_set((value & (1 << bit)) == 0);
    sf_set((value & (1 << bit)) != 0 && bit == 7);
    alu_parity(value & (1 << bit));
    xf_set(value & (1 << 3));
    yf_set(value & (1 << 5));
}

// [English] Logical XOR byte
// [Portuguese] XOR lógico entre bytes
uint8_t alu_xor_byte(uint8_t value1, uint8_t value2)
{
    cf_set(0);
    nf_set(0);
    hf_set(0);
    value1 ^= value2;
    alu_parity(value1);
    zf_set(value1 == 0);
    sf_set(value1 & 128);
    xf_set(value1 & (1 << 3));
    yf_set(value1 & (1 << 5));
    return value1;
}

// [English] Logical OR byte
// [Portuguese] OR lógico entre bytes
uint8_t alu_or_byte(uint8_t value1, uint8_t value2)
{
    cf_set(0);
    nf_set(0);
    hf_set(0);
    value1 |= value2;
    alu_parity(value1);
    zf_set(value1 == 0);
    sf_set(value1 & 128);
    xf_set(value1 & (1 << 3));
    yf_set(value1 & (1 << 5));
    return value1;
}

// [English] Calculate parity flag
// [Portuguese] Calcula a flag de paridade
void alu_parity(uint8_t value)
{
    // [English] Count bits and set P/V to true if even
    // [Portuguese] Conta os bits e define P/V como verdadeiro se par
    int bits = 0;
    for (size_t i = 0; i < 8; i++)
    {
        bits += ((value >> i) & 1) ? 1 : 0;
    }
    pvf_set((bits & 1) == 0);
}

// [English] Decimal adjust accumulator (DAA)
// [Portuguese] Ajuste decimal do acumulador
uint8_t alu_daa(uint8_t value)
{
    // [English] If subtracting, adjust down; if adding, adjust up for BCD
    // [Portuguese] Se subtraindo, ajusta para baixo; se somando, ajusta para cima para BCD
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
    xf_set(value & (1 << 3));
    yf_set(value & (1 << 5));
    alu_parity(value);
    return value;
}
