global puts
puts:
    push b
    lxi h, 4
    dad sp
    mov e, m
    inx h
    mov d, m
    mvi c, 9
    call 5
    pop b
    ret
