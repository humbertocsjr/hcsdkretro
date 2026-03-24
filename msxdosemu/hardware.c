#include "emu.h"

void hardware_out(uint8_t port, uint8_t value)
{

}

uint8_t hardware_in(uint8_t port)
{
    uint8_t value = 0;
    alu_parity(value);
    nf_set(0);
    hf_set(0);
    zf_set(value == 0);
    sf_set(value & 128);
    return 0;
}