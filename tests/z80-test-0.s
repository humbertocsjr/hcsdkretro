section text
    db 0xeb, 0xfe, 0x90
    times 0xc01e - $ db 0x00
global _start
_start:
    ld sp, 0xd000
    ld hl, [0xf351]
    inc h
    ld bc, 256
    ld de, 0xc100
    ldir

