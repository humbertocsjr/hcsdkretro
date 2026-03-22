def tty_print_char(c as char)
    asm z80 "ld hl, 2"
    asm z80 "add hl, sp"
    asm z80 "ld a, [hl]"
    asm z80 "ld e, a"
    asm z80 "ld c, 2"
    asm z80 "call 5"
end