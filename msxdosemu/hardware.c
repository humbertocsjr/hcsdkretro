#include "emu.h"

static uint8_t _psg_register = 0;
static uint8_t _psg_regs[16];
static uint8_t _ppi_port_a = 0;
static uint8_t _ppi_port_c = 0;
static uint8_t _primary_slot = 0;

// [English] Update Z80 flags based on an input value
// [Portuguese] Atualiza flags Z80 com base no valor lido
static void flags_update(uint8_t value)
{
    alu_parity(value);
    nf_set(0);
    hf_set(0);
    zf_set(value == 0);
    sf_set(value & 128);
}

// [English] Write to MSX I/O port
// [Portuguese] Escreve em uma porta de I/O do MSX
void hardware_out(uint8_t port, uint8_t value)
{
    switch(port)
    {
        case 0x98: screen_out_98(value); break;
        case 0x99: screen_out_99(value); break;
        case 0xA0: _psg_register = value; break;
        case 0xA1: _psg_regs[_psg_register & 0x0F] = value; break;
        case 0xA2: _ppi_port_a = value; break;
        case 0xA8: _primary_slot = value; break;
        default:
            fprintf(stderr, "[WARNING: UNIMPLEMENTED OUTPUT PORT 0x%02X]\n", port);
            break;
    }
}

// [English] Read from MSX I/O port
// [Portuguese] Lê de uma porta de I/O do MSX
uint8_t hardware_in(uint8_t port)
{
    // [English] Read value from appropriate hardware port and update flags
    // [Portuguese] Lê o valor da porta de hardware apropriada e atualiza flags
    uint8_t value = 0;
    switch(port)
    {
        case 0x98: value = screen_in_98(); break;
        case 0x99: value = screen_in_99(); flags_update(value); return value;
        case 0xA0: value = _psg_regs[_psg_register & 0x0F]; break;
        case 0xA2: value = _ppi_port_c; break;
        case 0xA8: value = _primary_slot; break;
        default:
            fprintf(stderr, "[WARNING: UNIMPLEMENTED INPUT PORT 0x%02X]\n", port);
            break;
    }
    flags_update(value);
    return value;
}