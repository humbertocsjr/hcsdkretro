global getchar
getchar:
    ld c, 1
    call 5
    ld l, a
    ld h, 0
    ret
