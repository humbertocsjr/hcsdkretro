global puts
puts:
    lxi h, 2
    dad sp
    mov e, m
    inx h
    mov d, m
    mvi c, 9
    call 5
    ret
