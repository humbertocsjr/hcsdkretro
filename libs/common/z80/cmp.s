section text

    global cmpe8
    cmpe8:
        cp e
        sbc a, a
        ret
    
    global cmpe16
    cmpe16:
        or a
        sbc hl, de
        ld hl, 0
        ret nz
        dec hl
        ret
    
    global cmpne8
    cmpne8:
        cp e
        sbc a, a
        cpl
        ret
    
    global cmpne16
    cmpne16:
        or a
        sbc hl, de
        ld hl, 0
        ret z
        dec hl
        ret
    
    global cmpa8
    cmpa8:
        cp e
        ccf          ; inverte carry
        sbc a, a     ; 0xFF se A >= E
        and 0xfe     ; zera caso igualdade
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
    
    global cmpb8
    cmpb8:
        cp e
        sbc a, a     ; 0xFF se carry (A < E)
        ret
    
    global cmpb16
    cmpb16:
        or a
        sbc hl, de
        ld hl, 0
        ret nc
        dec hl
        ret
    
    global cmpae8
    cmpae8:
        cp e
        ccf
        sbc a, a     ; 0xFF se >=
        ret
    
    global cmpae16
    cmpae16:
        or a
        sbc hl, de
        ld hl, 0
        ret c
        dec hl
        ret
    
    global cmpbe8
    cmpbe8:
        cp e
        sbc a, a     ; 0xFF se <
        cpl          ; inverte para <=
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
    
    global cmpg8
    cmpg8:
        cp e
        ret z
        jp pe, .no_overflow
        jp m, .true
        xor a
        ret
        .no_overflow:
            jp p, .true
            xor a
            ret
        .true:
            ld a, 0xff
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
    
    global cmpl8
    cmpl8:
        cp e
        jp pe, .no_overflow
        jp p, .true
        xor a
        ret
        .no_overflow:
            jp m, .true
            xor a
            ret
        .true:
            ld a, 255
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
    
    global cmpge8
    cmpge8:
        cp e
        jr z, .true
        jp pe, .no_overflow
        jp m, .true
        xor a
        ret
        .no_overflow:
            jp p, .true
            xor a
            ret
        .true:
            ld a, 255
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
    
    global cmple8
    cmple8:
        cp e
        jr z, .true
        jp pe, .no_overlow
        jp p, .true
        xor a
        ret
        .no_overlow:
            jp m, .true
            xor a
            ret
        .true:
            ld a, 255
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
    
    