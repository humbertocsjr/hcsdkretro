#include "emu.h"


void cf_set(bool value)
{
    _regs_curr.af.f &= ~1;
    _regs_curr.af.f |= value ? 1 : 0;
}

bool cf_get()
{
    return _regs_curr.af.f & 1;
}

void xf_set(bool value)
{
    _regs_curr.af.f &= ~8;
    _regs_curr.af.f |= value ? 8 : 0;
}

bool xf_get()
{
    return _regs_curr.af.f & 8;
}

void yf_set(bool value)
{
    _regs_curr.af.f &= ~32;
    _regs_curr.af.f |= value ? 32 : 0;
}

bool yf_get()
{
    return _regs_curr.af.f & 32;
}

void nf_set(bool value)
{
    _regs_curr.af.f &= ~2;
    _regs_curr.af.f |= value ? 2 : 0;
}

bool nf_get()
{
    return _regs_curr.af.f & 2;
}

void pvf_set(bool value)
{
    _regs_curr.af.f &= ~4;
    _regs_curr.af.f |= value ? 4 : 0;
}

bool pvf_get()
{
    return _regs_curr.af.f & 4;
}

void hf_set(bool value)
{
    _regs_curr.af.f &= ~16;
    _regs_curr.af.f |= value ? 16 : 0;
}

bool hf_get()
{
    return _regs_curr.af.f & 16;
}

void zf_set(bool value)
{
    _regs_curr.af.f &= ~64;
    _regs_curr.af.f |= value ? 64 : 0;
}

bool zf_get()
{
    return _regs_curr.af.f & 64;
}

void sf_set(bool value)
{
    _regs_curr.af.f &= ~128;
    _regs_curr.af.f |= value ? 128 : 0;
}

bool sf_get()
{
    return _regs_curr.af.f & 128;
}