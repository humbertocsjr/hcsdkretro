#include "emu.h"

// [English] MSX BIOS reboot (HALT)
// [Portuguese] Reinicialização via BIOS MSX (HALT)
void abi_msxbios_reboot()
{
    _executing = false;
}

// [English] MSX BIOS Read Slot (RDSLT) - not implemented
// [Portuguese] Leitura de slot da BIOS MSX - não implementado
void abi_msxbios_rdslt()
{
    fprintf(stderr, "\n\n[ERROR: RDSLT NOT IMPLEMENTED; PREV IP=0x%04X; CURR IP=0x%04X]\n", _regs_prev.ip, _regs_curr.ip);
    exit(1);
}

// [English] MSX BIOS Write Slot (WRSLT) - not implemented
// [Portuguese] Escrita em slot da BIOS MSX - não implementado
void abi_msxbios_wrslt()
{
    fprintf(stderr, "\n\n[ERROR: WRSLT NOT IMPLEMENTED; PREV IP=0x%04X; CURR IP=0x%04X]\n", _regs_prev.ip, _regs_curr.ip);
    exit(1);
}

// [English] MSX BIOS Call Slot (CALSLT) - not implemented
// [Portuguese] Chamada de slot da BIOS MSX - não implementado
void abi_msxbios_callslt()
{
    fprintf(stderr, "\n\n[ERROR: CALLSLT NOT IMPLEMENTED; PREV IP=0x%04X; CURR IP=0x%04X]\n", _regs_prev.ip, _regs_curr.ip);
    exit(1);
}

// [English] MSX BIOS Enable Slot (ENASLT) - not implemented
// [Portuguese] Habilitar slot da BIOS MSX - não implementado
void abi_msxbios_enaslt()
{
    fprintf(stderr, "\n\n[ERROR: ENASLT NOT IMPLEMENTED; PREV IP=0x%04X; CURR IP=0x%04X]\n", _regs_prev.ip, _regs_curr.ip);
    exit(1);
}

// [English] MSX BIOS Call Far (CALLF) - not implemented
// [Portuguese] Chamada far da BIOS MSX - não implementado
void abi_msxbios_callf()
{
    fprintf(stderr, "\n\n[ERROR: CALLF NOT IMPLEMENTED; PREV IP=0x%04X; CURR IP=0x%04X]\n", _regs_prev.ip, _regs_curr.ip);
    exit(1);
}