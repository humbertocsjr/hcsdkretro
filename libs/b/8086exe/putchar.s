; 32-bit (far pointer) putchar for MZ EXE
; Parameter: [bp+4] = 32-bit value (only low byte used)
global putchar
section text
putchar:
    push bp
    mov bp, sp
    mov dl, [bp+4]
    mov ah, 2
    int 0x21
    pop bp
    ret
