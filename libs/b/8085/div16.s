section text
    global __div16
__div16:
    push b
    xchg
    lxi b, 0
    mvi a, 16
.l:
    push psw
    dad h
    mov a, c
    ral
    mov c, a
    mov a, b
    ral
    mov b, a
    mov a, c
    sub e
    mov c, a
    mov a, b
    sbb d
    mov b, a
    jc .restore
    inx h
    jmp .next
.restore:
    mov a, c
    add e
    mov c, a
    mov a, b
    adc d
    mov b, a
.next:
    pop psw
    dcr a
    jnz .l
    pop b
    ret

    global __mod16
__mod16:
    push b
    xchg
    lxi b, 0
    mvi a, 16
.l:
    push psw
    dad h
    mov a, c
    ral
    mov c, a
    mov a, b
    ral
    mov b, a
    mov a, c
    sub e
    mov c, a
    mov a, b
    sbb d
    mov b, a
    jc .restore
    inx h
    jmp .next
.restore:
    mov a, c
    add e
    mov c, a
    mov a, b
    adc d
    mov b, a
.next:
    pop psw
    dcr a
    jnz .l
    mov l, c
    mov h, b
    pop b
    ret
