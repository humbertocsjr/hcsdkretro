#include "emu.h"


void abi_dos_call_5()
{
    uint16_t ptr;
    switch(_regs_curr.bc.c)
    {
        case 0x00: // Program terminate
            _executing = false;
            break;
        case 0x02: // Console output
            screen_put_char(_regs_curr.de.e);
            break;
        case 0x09: // String output
            ptr = _regs_curr.de.word;
            while(mem_get_byte(ptr) != '$')
            {
                screen_put_char(mem_get_byte(ptr++));
            }
            break;
        case 0x0c: // Return Version Number (CP/M)
            _regs_curr.af.a = 0x22;
            _regs_curr.hl.l = 0x22;
            _regs_curr.bc.b = 0x00;
            _regs_curr.hl.h = 0x00;
            break;
        case 0x6f: // Get MSX-DOS Version Number
            _regs_curr.af.a = 0;
            _regs_curr.bc.word = 0x0100;
            _regs_curr.de.word = 0x0000;
        default:
            fprintf(stderr, "\n\n[ERROR: NOT IMPLEMENTED COMMAND %i OF CALL 5 ABI]", _regs_curr.bc.c);
            exit(1);
            break;
    }

    _regs_curr.ip = mem_get_word(_regs_curr.sp);
    _regs_curr.sp += 2;
}