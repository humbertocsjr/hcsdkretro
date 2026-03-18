def tty_print_char(c as char)
    asm 8086 "mov bx, sp"
    asm 8086 "mov dl, [bx+2]"
    asm 8086 "mov ah, 2"
    asm 8086 "int 0x21"
end