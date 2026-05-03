global putchar
putchar:
    push b
    lxi h, 4
    dad sp
    mov e, m
    mvi c, 2
    call 5
    pop b
    ret
