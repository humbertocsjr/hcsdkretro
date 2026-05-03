global bdos
bdos:
    push b
    lxi h, 4
    dad sp
    mov c, m
    lxi h, 6
    dad sp
    mov e, m
    inx h
    mov d, m
    call 5
    mov l, a
    mvi h, 0
    pop b
    ret

global set_dma
set_dma:
    push b
    lxi h, 4
    dad sp
    mov e, m
    inx h
    mov d, m
    mvi c, 26
    call 5
    pop b
    ret
