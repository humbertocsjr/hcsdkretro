global puts
puts:
    ld hl, 2
    add hl, sp
    ld e, [hl]
    inc hl
    ld d, [hl]
    ld c, 9
    call 5
    ret
