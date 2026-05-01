global putchar
putchar:
    ld hl, 2
    add hl, sp
    ld e, [hl]
    ld c, 2
    call 5
    ret
