global puts
puts:
    push bp
    mov bp, sp
    mov dx, [bp+4]
    mov ah, 9
    int 0x21
    pop bp
    ret
