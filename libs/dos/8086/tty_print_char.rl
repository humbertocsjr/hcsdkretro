def tty_print_char(c as char)
    asm 8086 "mov dl, [bp+4]"
    asm 8086 "mov ah, 2"
    asm 8086 "int 0x21"
end