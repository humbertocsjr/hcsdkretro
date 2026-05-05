#include "emu.h"

// [English] Read a byte from emulated memory
// [Portuguese] Lê um byte da memória emulada
uint8_t mem_get_byte(uint16_t address)
{
    return _memory[address];
}

// [English] Write a byte to emulated memory
// [Portuguese] Escreve um byte na memória emulada
void mem_set_byte(uint16_t address, uint8_t value)
{
    _memory[address] = value;
}

// [English] Read a 16-bit word from emulated memory (little-endian)
// [Portuguese] Lê uma word (16 bits) em little-endian
uint16_t mem_get_word(uint16_t address)
{
    return _memory[address] | (_memory[address + 1] << 8);
}

// [English] Write a 16-bit word to emulated memory (little-endian)
// [Portuguese] Escreve uma word (16 bits) em little-endian
void mem_set_word(uint16_t address, uint16_t value)
{
    _memory[address] = value & 0xff;
    _memory[address+1] = (value >> 8) & 0xff;
}

// [English] Fetch and return next byte from instruction pointer
// [Portuguese] Busca e retorna o próximo byte do IP
uint8_t ip_get_byte()
{
    return _memory[_regs_curr.ip++];
}

// [English] Fetch next byte as signed value from instruction pointer
// [Portuguese] Busca próximo byte como valor signed do IP
int8_t ip_get_byte_signed()
{
    return *(int8_t *)&_memory[_regs_curr.ip++];
}

// [English] Fetch next 16-bit word from instruction pointer
// [Portuguese] Busca a próxima word (16 bits) do IP
uint16_t ip_get_word()
{
    uint16_t val = _memory[_regs_curr.ip++];
    val |= _memory[_regs_curr.ip++] << 8;
    return val;
}