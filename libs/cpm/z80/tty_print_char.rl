def tty_print_char(c as char)
    asm z80 "ld e, [ix+4]"
    asm z80 "ld c, 2"
    asm z80 "call 5"
end