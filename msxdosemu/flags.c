#include "emu.h"


// [English] Set Carry Flag
// [Portuguese] Define a flag de Carry
void cf_set(bool value)
{
    _regs_curr.af.f &= ~1;
    _regs_curr.af.f |= value ? 1 : 0;
}

// [English] Get Carry Flag
// [Portuguese] Obtém a flag de Carry
bool cf_get()
{
    return _regs_curr.af.f & 1;
}

// [English] Set X Flag (bit 3 of F)
// [Portuguese] Define a flag X (bit 3 de F)
void xf_set(bool value)
{
    _regs_curr.af.f &= ~8;
    _regs_curr.af.f |= value ? 8 : 0;
}

// [English] Get X Flag
// [Portuguese] Obtém a flag X
bool xf_get()
{
    return _regs_curr.af.f & 8;
}

// [English] Set Y Flag (bit 5 of F)
// [Portuguese] Define a flag Y (bit 5 de F)
void yf_set(bool value)
{
    _regs_curr.af.f &= ~32;
    _regs_curr.af.f |= value ? 32 : 0;
}

// [English] Get Y Flag
// [Portuguese] Obtém a flag Y
bool yf_get()
{
    return _regs_curr.af.f & 32;
}

// [English] Set Add/Subtract Flag (N flag, bit 1)
// [Portuguese] Define a flag N (soma/subtração, bit 1)
void nf_set(bool value)
{
    _regs_curr.af.f &= ~2;
    _regs_curr.af.f |= value ? 2 : 0;
}

// [English] Get Add/Subtract Flag
// [Portuguese] Obtém a flag N
bool nf_get()
{
    return _regs_curr.af.f & 2;
}

// [English] Set Parity/Overflow Flag (bit 2 of F)
// [Portuguese] Define a flag P/V (paridade/overflow, bit 2)
void pvf_set(bool value)
{
    _regs_curr.af.f &= ~4;
    _regs_curr.af.f |= value ? 4 : 0;
}

// [English] Get Parity/Overflow Flag
// [Portuguese] Obtém a flag P/V
bool pvf_get()
{
    return _regs_curr.af.f & 4;
}

// [English] Set Half-Carry Flag (bit 4 of F)
// [Portuguese] Define a flag H (half-carry, bit 4)
void hf_set(bool value)
{
    _regs_curr.af.f &= ~16;
    _regs_curr.af.f |= value ? 16 : 0;
}

// [English] Get Half-Carry Flag
// [Portuguese] Obtém a flag H
bool hf_get()
{
    return _regs_curr.af.f & 16;
}

// [English] Set Zero Flag (bit 6 of F)
// [Portuguese] Define a flag Z (zero, bit 6)
void zf_set(bool value)
{
    _regs_curr.af.f &= ~64;
    _regs_curr.af.f |= value ? 64 : 0;
}

// [English] Get Zero Flag
// [Portuguese] Obtém a flag Z
bool zf_get()
{
    return _regs_curr.af.f & 64;
}

// [English] Set Sign Flag (bit 7 of F)
// [Portuguese] Define a flag S (sinal, bit 7)
void sf_set(bool value)
{
    _regs_curr.af.f &= ~128;
    _regs_curr.af.f |= value ? 128 : 0;
}

// [English] Get Sign Flag
// [Portuguese] Obtém a flag S
bool sf_get()
{
    return _regs_curr.af.f & 128;
}