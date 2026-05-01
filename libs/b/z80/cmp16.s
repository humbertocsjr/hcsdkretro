section text

    global cmpe16
    cmpe16:
        or a
        sbc hl, de
        ld hl, 0
        ret nz
        dec hl
        ret

    global cmpne16
    cmpne16:
        or a
        sbc hl, de
        ld hl, 0
        ret z
        dec hl
        ret

    global cmpa16
    cmpa16:
        or a
        sbc hl, de
        ld hl, 0
        ret c
        ret z
        dec hl
        ret

    global cmpb16
    cmpb16:
        or a
        sbc hl, de
        ld hl, 0
        ret nc
        dec hl
        ret

    global cmpae16
    cmpae16:
        or a
        sbc hl, de
        ld hl, 0
        ret c
        dec hl
        ret

    global cmpbe16
    cmpbe16:
        or a
        sbc hl, de
        ld hl, 0
        ret c
        ret z
        dec hl
        ret

    global cmpg16
    cmpg16:
        or a
        sbc hl, de
        ld hl, 0
        ret z
        jp pe, .no_overlow
        jp m, .true
        ret
        .no_overlow:
            jp p, .true
            ret
        .true:
            dec hl
            ret

    global cmpl16
    cmpl16:
        or a
        sbc hl, de
        ld hl, 0
        jp pe, .no_overlow
        jp p, .true
        ret
        .no_overlow:
            jp m, .true
            ret
        .true:
            dec hl
            ret

    global cmpge16
    cmpge16:
        or a
        sbc hl, de
        ld hl, 0
        ret z
        jp pe, .no_overlow
        jp m, .true
        ret
        .no_overlow:
            jp p, .true
            ret
        .true:
            dec hl
            ret

    global cmple16
    cmple16:
        or a
        sbc hl, de
        ld hl, 0
        ret z
        jp pe, .no_overlow
        jp m, .true
        ret
        .no_overlow:
            jp p, .true
            ret
        .true:
            dec hl
            ret
    