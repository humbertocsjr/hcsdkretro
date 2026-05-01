global getchar
getchar:
    push bp
    mov bp, sp
    mov ah, 1
    int 0x21
    mov ah, 0
    pop bp
    ret
