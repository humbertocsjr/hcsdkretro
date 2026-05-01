section text
    global muls8
    global muls16
    global mulu8
    global mulu16
muls8:
mulu8:
    ld b, 0
    ld a, e
    and 0x80
    jr z, muls16
    ld a, e
    neg
    ld e, a
    inc b
muls16:
mulu16:
    ld hl, 0
    ld a, 8
.l:
    add hl, hl
    jr nc, .skip
    add hl, de
.skip:
    dec a
    jr nz, .l
    ld a, b
    and 1
    ret z
    ex de, hl
    ld hl, 0
    or a
    sbc hl, de
    ret
