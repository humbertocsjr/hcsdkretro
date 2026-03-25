#include "emu.h"

void hardware_out(uint8_t port, uint8_t value)
{
    fprintf(stderr, "\n\n[ERROR: NOT IMPLEMENTED OUTPUT PORT %i]", port);
    exit(1);
}

uint8_t hardware_in(uint8_t port)
{
    uint8_t value = 0;
    alu_parity(value);
    nf_set(0);
    hf_set(0);
    zf_set(value == 0);
    sf_set(value & 128);

    fprintf(stderr, "\n\n[ERROR: NOT IMPLEMENTED INPUT PORT %i]", port);
    exit(1);
    return 0;
}