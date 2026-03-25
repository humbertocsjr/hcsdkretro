#include "emu.h"


void abi_dos_call_5()
{
    switch(_regs_curr.bc.c)
    {
        case 0:
            _executing = false;
            break;
        case 2:
            screen_put_char(_regs_curr.de.e);
            break;
        default:
            fprintf(stderr, "\n\n[ERROR: NOT IMPLEMENTED COMMAND %i OF CALL 5 ABI]", _regs_curr.bc.c);
            exit(1);
            break;
    }

    _regs_curr.ip = mem_get_word(_regs_curr.sp);
    _regs_curr.sp += 2;
}