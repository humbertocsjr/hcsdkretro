global bdos
bdos:
    lxi h, 2
    dad sp
    mov c, m
    lxi h, 4
    dad sp
    mov e, m
    inx h
    mov d, m
    call 5
    mov l, a
    mvi h, 0
    ret

global set_dma
set_dma:
    lxi h, 2
    dad sp
    mov e, m
    inx h
    mov d, m
    mvi c, 26
    call 5
    ret
