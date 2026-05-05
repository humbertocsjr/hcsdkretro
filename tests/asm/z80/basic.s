;; Z80 assembler test - basic instructions
section text
global _start
_start:
    ; 8-bit load
    ld a, 0x42
    ld b, a
    ld c, 0x10
    ld d, c
    ld e, 0xFF
    ld h, d
    ld l, e

    ; 16-bit load
    ld hl, 0x1234
    ld de, 0x5678
    ld bc, 0x9ABC
    ld sp, 0xFFFE
    push hl
    pop de

    ; 8-bit arithmetic
    add a, 1
    adc a, 0
    sub 1
    sbc a, 0
    and 0x0F
    or 0xF0
    xor 0xFF
    cp 0x42
    inc a
    dec a

    ; 16-bit arithmetic
    add hl, de
    sbc hl, de
    adc hl, bc

    ; shifts
    rlca
    rrca
    rla
    rra

    ; exchange
    ex de, hl

    ; block
    ldi
    ldd
    cpi

    ; jump / call / ret
    jp .L1
    jr .L1
    call .L1
    ret

.L1:
    ; indexed (ix)
    ld a, [ix+0]
    ld [ix+1], a
    ld l, [ix-2]
    ld h, [ix-1]
    ld [ix+4], e
    ld [ix+5], d

    ; bit operations
    set 0, a
    res 1, b
    bit 2, c

    ret
