#include "emu.h"

uint8_t mem_get_byte(uint16_t address)
{
    return _memory[address];
}

void mem_set_byte(uint16_t address, uint8_t value)
{
    _memory[address] = value;
}

uint16_t mem_get_word(uint16_t address)
{
    return _memory[address] | (_memory[address + 1] << 8);
}

void mem_set_word(uint16_t address, uint16_t value)
{
    _memory[address] = value & 0xff;
    _memory[address+1] = (value >> 8) * 0xff;
}

uint8_t ip_get_byte()
{
    return _memory[_regs_curr.ip++];
}

int8_t ip_get_byte_signed()
{
    return *(int8_t *)&_memory[_regs_curr.ip++];
}

uint16_t ip_get_word()
{
    uint16_t val = _memory[_regs_curr.ip++];
    val |= _memory[_regs_curr.ip++] << 8;
    return val;
}