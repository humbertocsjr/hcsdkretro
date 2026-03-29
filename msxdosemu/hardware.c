#include "emu.h"

void hardware_out(uint8_t port, uint8_t value)
{
    switch(port)
    {
        case 0x98: screen_out_98(value); break;
        case 0x99: screen_out_99(value); break;
        default:
            fprintf(stderr, "\n\n[ERROR: NOT IMPLEMENTED OUTPUT PORT %i]", port);
            exit(1);
            break;
    }
}

uint8_t hardware_in(uint8_t port)
{
    uint8_t value = 0;
    switch(port)
    {
        case 0x98: value = screen_in_98(); break;
        case 0x99: value = screen_in_99(); break;
    }
    alu_parity(value);
    nf_set(0);
    hf_set(0);
    zf_set(value == 0);
    sf_set(value & 128);

    fprintf(stderr, "\n\n[ERROR: NOT IMPLEMENTED INPUT PORT %i]", port);
    exit(1);
    return 0;
}