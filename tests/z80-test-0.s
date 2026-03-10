section text
global _start
_start:
    ld sp, 0xd000
    ld hl, [0xf351]
    inc h
    ld bc, 256
    ld de, 0xc100
    ldir
    jr _start

skip_spaces:
peek:
next:
mul16:
    parse_num:
        call skip_spaces

        ld hl, 0
        ld b, 0

        .loop:
            call peek
            cp '0'
            jr c, .end
            cp '9' + 1
            jr nc, .end

            inc b

            call next
            sub '0'

            push af
            push hl

            ld de, 10
            call mul16

            pop de
            pop af

            ld e, a
            ld d, 0
            add hl, de

            jr .loop
        .end:
            ld a, b
            or a
            jp z, .error
            or a
            ret
        .error: