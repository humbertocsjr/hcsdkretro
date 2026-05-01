section text

    global divs8
    divs8:
        ld b, a
        xor e
        push af
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
            call divu8

            pop af
            jp p, .done
            neg
        .done:
        ret

    global divu8
    divu8:
    ld b, 0        ; quociente
    ld c, a        ; resto parcial
    ld a, 8
    .loop:
        sla c
        rl b
        cp e
        jr c, .skip
        sub e
        inc b
    .skip:
        dec a
        jr nz, .loop
    ld a, b
    ld e, c
    ret
    
    global mods8
    mods8:
        call divs8
        ld a, e
        ret

    global movu8
    modu8:
        call divu8
        ld a, e
        ret
    