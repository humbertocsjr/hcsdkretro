global bdos
bdos:
    push ix
    ld ix, 0
    add ix, sp
    ld c, [ix+4]
    ld e, [ix+6]
    ld d, [ix+7]
    call 5
    ld l, a
    ld h, 0
    pop ix
    ret

global set_dma
set_dma:
    push ix
    ld ix, 0
    add ix, sp
    ld e, [ix+4]
    ld d, [ix+5]
    ld c, 26
    call 5
    pop ix
    ret
