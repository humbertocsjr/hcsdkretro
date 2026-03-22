section text

    global cmpe8
    cmpe8:
        cp e
        sbc a, a
        ret
    
    
    global cmpne8
    cmpne8:
        cp e
        sbc a, a
        cpl
        ret
    
    
    global cmpa8
    cmpa8:
        cp e
        ccf          ; inverte carry
        sbc a, a     ; 0xFF se A >= E
        and 0xfe     ; zera caso igualdade
        ret
    
    
    global cmpb8
    cmpb8:
        cp e
        sbc a, a     ; 0xFF se carry (A < E)
        ret
    
    
    global cmpae8
    cmpae8:
        cp e
        ccf
        sbc a, a     ; 0xFF se >=
        ret
    
    
    global cmpbe8
    cmpbe8:
        cp e
        sbc a, a     ; 0xFF se <
        cpl          ; inverte para <=
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
    
    