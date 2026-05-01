global putchar
putchar:
    push bp
    mov bp, sp
    mov dl, [bp+4]
    mov ah, 2
    int 0x21
    pop bp
    ret
