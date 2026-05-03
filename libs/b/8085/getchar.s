global getchar
getchar:
    push b
    mvi c, 1
    call 5
    mov l, a
    mvi h, 0
    pop b
    ret
