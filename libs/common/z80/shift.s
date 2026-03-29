section text
    global shl8
    shl8:
        ld b, e
        ld c, a
        ld a, b
        or a
        ld a, c
        ret z
        .loop:
            add a, a
            djnz .loop
        ret
    global shl16
    shl16:
        ld a, e
        or a
        ret z
        ld b, e
        .loop:
            add hl, hl
            djnz .loop
        ret

    global shr8
    shr8:
        ld b, e
        ld c, a
        ld a, b
        or a
        ld a, c
        ret z
        .loop:
            srl a
            djnz .loop
        ret
    global shr16
    shr16:
        ld a, e
        or a
        ret z
        ld b, e
        .loop:
            srl h
            rr l
            djnz .loop
        ret