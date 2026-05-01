global exit
exit:
    push bp
    mov bp, sp
    mov al, [bp+4]
    mov ah, 0x4C
    int 0x21
    pop bp
    ret
