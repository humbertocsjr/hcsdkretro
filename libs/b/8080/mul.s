section text
    global __mul16
__mul16:
    push b
    xchg
    push h
    pop b
    lxi h, 0
    mvi a, 16
.l:
    push psw
    mov a, b
    rar
    mov b, a
    mov a, c
    rar
    mov c, a
    jnc .skip
    dad d
.skip:
    xchg
    dad h
    xchg
    pop psw
    dcr a
    jnz .l
    pop b
    ret
