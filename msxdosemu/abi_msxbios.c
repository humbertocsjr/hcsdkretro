#include "emu.h"

void abi_msxbios_reboot()
{
    _executing = false;
}

void abi_msxbios_rdslt()
{
    fprintf(stderr, "\n\n[ERROR: RDSLT NOT IMPLEMENTED]\n");
    exit(1);
}

void abi_msxbios_wrslt()
{
    fprintf(stderr, "\n\n[ERROR: WRSLT NOT IMPLEMENTED]\n");
    exit(1);
}

void abi_msxbios_callslt()
{
    fprintf(stderr, "\n\n[ERROR: CALLSLT NOT IMPLEMENTED]\n");
    exit(1);
}

void abi_msxbios_enaslt()
{
    fprintf(stderr, "\n\n[ERROR: ENASLT NOT IMPLEMENTED]\n");
    exit(1);
}

void abi_msxbios_callf()
{
    fprintf(stderr, "\n\n[ERROR: CALLF NOT IMPLEMENTED]\n");
    exit(1);
}