section text
    db 0xeb, 0xfe, 0x90
    times 0x11e - $ db 0x00
global _start
_start:
    mov sp, 0xd000
    mov ax, [0xf351]
    inc al
    mov cx, 256
    mov cx, 0xc100
    movsb
    mov ax, [bp+1]

