section text

    divs16:
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
            call divu16
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


    divu16:
        ld bc, 0        ; quociente
        ld a, 16
        .loop:
            add hl, hl
            rl c
            rl b
            or a
            sbc hl, de
            jr c, .restore
            inc bc
            jr .next
        .restore:
            add hl, de
        .next:
            dec a
            jr nz, .loop
        ex de, hl       ; resto
        ld h, b
        ld l, c         ; quociente
        ret

    mods16:
        call divs16
        ex de, hl
        ret

    modu16:
        call divu16
        ex de, hl
        ret