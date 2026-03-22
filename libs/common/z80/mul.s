section text

    global muls8
    muls8:
        ld b, a
        xor e
        push af        ; salva sinal
        bit 7, b
        jr z, .a_pos
        neg
        .a_pos:
            ld a, e
            bit 7, a
            jr z, .e_pos
            neg
        .e_pos:
            ld e, a
            ld a, b
            call mulu8
            pop af
            jp p, .done
            neg
        .done:
        ret

    global muls16
    muls16:
        ld a, h
        xor d
        push af
        bit 7, h
        jr z, .hl_pos
        xor a
        sub l
        ld l, a
        sbc a, a
        sub h
        ld h, a
        .hl_pos:
            bit 7, d
            jr z, .de_pos
            xor a
            sub e
            ld e, a
            sbc a, a
            sub d
            ld d, a
        .de_pos:
            call mulu16
            pop af
            jp p, .done
            xor a
            sub l
            ld l, a
            sbc a, a
            sub h
            ld h, a
        .done:
        ret
        
    global mulu8
    mulu8:
        ld d, 0        ; acumulador alto (ignorado)
        ld b, 8
        .loop:
            srl e      ; bit -> carry
            jr nc, .skip
            add a, d   ; soma parcial
            .skip:
            sla d      ; desloca acumulador
            djnz .loop
        ret
    
    global mulu16
    mulu16:
        ld bc, 0
        ld a, 16
        .loop:
            add hl, hl
            rl e
            rl d
            jr nc, .skip
            add hl, bc
            .skip:
            dec a
            jr nz, .loop
        ret
        