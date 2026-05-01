global putchar
putchar:
    lxi h, 2
    dad sp
    mov e, m
    mvi c, 2
    call 5
    ret
