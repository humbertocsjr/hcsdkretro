section text
global _start
_start:
    ; === MAIN INSTRUCTIONS (00-FF) ===
    nop                             ; 00
    ld bc, 0x1234                   ; 01
    ld [bc], a                      ; 02
    inc bc                          ; 03
    inc b                           ; 04
    dec b                           ; 05
    ld b, 0x42                      ; 06
    rlca                            ; 07
    ex af, af'                      ; 08
    add hl, bc                      ; 09
    ld a, [bc]                      ; 0a
    dec bc                          ; 0b
    inc c                           ; 0c
    dec c                           ; 0d
    ld c, 0x42                      ; 0e
    rrca                            ; 0f

    djnz _start                     ; 10
    ld de, 0x1234                   ; 11
    ld [de], a                      ; 12
    inc de                          ; 13
    inc d                           ; 14
    dec d                           ; 15
    ld d, 0x42                      ; 16
    rla                             ; 17
    jr _start                       ; 18
    add hl, de                      ; 19
    ld a, [de]                      ; 1a
    dec de                          ; 1b
    inc e                           ; 1c
    dec e                           ; 1d
    ld e, 0x42                      ; 1e
    rra                             ; 1f

    jr nz, _start                   ; 20
    ld hl, 0x1234                   ; 21
    ld [0x5678], hl                 ; 22
    inc hl                          ; 23
    inc h                           ; 24
    dec h                           ; 25
    ld h, 0x42                      ; 26
    daa                             ; 27
    jr z, _start                    ; 28
    add hl, hl                      ; 29
    ld hl, [0x5678]                 ; 2a
    dec hl                          ; 2b
    inc l                           ; 2c
    dec l                           ; 2d
    ld l, 0x42                      ; 2e
    cpl                             ; 2f

    jr nc, _start                   ; 30
    ld sp, 0xFFFE                   ; 31
    ld [0x5678], a                  ; 32
    inc sp                          ; 33
    inc [hl]                        ; 34
    dec [hl]                        ; 35
    ld [hl], 0x42                   ; 36
    scf                             ; 37
    jr c, _start                    ; 38
    add hl, sp                      ; 39
    ld a, [0x5678]                  ; 3a
    dec sp                          ; 3b
    inc a                           ; 3c
    dec a                           ; 3d
    ld a, 0x42                      ; 3e
    ccf                             ; 3f

    ; LD r, r' (40-7F)
    ld b, b                         ; 40
    ld b, c                         ; 41
    ld b, d                         ; 42
    ld b, e                         ; 43
    ld b, h                         ; 44
    ld b, l                         ; 45
    ld b, [hl]                      ; 46
    ld b, a                         ; 47
    ld c, b                         ; 48
    ld c, d                         ; 4a
    ld c, e                         ; 4b
    ld c, h                         ; 4c
    ld c, l                         ; 4d
    ld c, [hl]                      ; 4e
    ld c, a                         ; 4f
    ld d, b                         ; 50
    ld d, c                         ; 51
    ld d, d                         ; 52
    ld d, e                         ; 53
    ld d, h                         ; 54
    ld d, l                         ; 55
    ld d, [hl]                      ; 56
    ld d, a                         ; 57
    ld e, b                         ; 58
    ld e, c                         ; 59
    ld e, d                         ; 5a
    ld e, e                         ; 5b
    ld e, h                         ; 5c
    ld e, l                         ; 5d
    ld e, [hl]                      ; 5e
    ld e, a                         ; 5f
    ld h, b                         ; 60
    ld h, c                         ; 61
    ld h, d                         ; 62
    ld h, e                         ; 63
    ld h, h                         ; 64
    ld h, l                         ; 65
    ld h, [hl]                      ; 66
    ld h, a                         ; 67
    ld l, b                         ; 68
    ld l, c                         ; 69
    ld l, d                         ; 6a
    ld l, e                         ; 6b
    ld l, h                         ; 6c
    ld l, l                         ; 6d
    ld l, [hl]                      ; 6e
    ld l, a                         ; 6f
    ld [hl], b                      ; 70
    ld [hl], c                      ; 71
    ld [hl], d                      ; 72
    ld [hl], e                      ; 73
    ld [hl], h                      ; 74
    ld [hl], l                      ; 75
    halt                            ; 76
    ld [hl], a                      ; 77
    ld a, b                         ; 78
    ld a, c                         ; 79
    ld a, d                         ; 7a
    ld a, e                         ; 7b
    ld a, h                         ; 7c
    ld a, l                         ; 7d
    ld a, [hl]                      ; 7e
    ld a, a                         ; 7f

    ; 8-bit ALU (80-BF)
    add a, b                        ; 80
    add a, c                        ; 81
    add a, d                        ; 82
    add a, e                        ; 83
    add a, h                        ; 84
    add a, l                        ; 85
    add a, [hl]                     ; 86
    add a, a                        ; 87
    adc a, b                        ; 88
    adc a, c                        ; 89
    adc a, d                        ; 8a
    adc a, e                        ; 8b
    adc a, h                        ; 8c
    adc a, l                        ; 8d
    adc a, [hl]                     ; 8e
    adc a, a                        ; 8f
    sub b                           ; 90
    sub c                           ; 91
    sub d                           ; 92
    sub e                           ; 93
    sub h                           ; 94
    sub l                           ; 95
    sub [hl]                        ; 96
    sub a                           ; 97
    sbc a, b                        ; 98
    sbc a, c                        ; 99
    sbc a, d                        ; 9a
    sbc a, e                        ; 9b
    sbc a, h                        ; 9c
    sbc a, l                        ; 9d
    sbc a, [hl]                     ; 9e
    sbc a, a                        ; 9f
    and b                           ; a0
    and c                           ; a1
    and d                           ; a2
    and e                           ; a3
    and h                           ; a4
    and l                           ; a5
    and [hl]                        ; a6
    and a                           ; a7
    xor b                           ; a8
    xor c                           ; a9
    xor d                           ; aa
    xor e                           ; ab
    xor h                           ; ac
    xor l                           ; ad
    xor [hl]                        ; ae
    xor a                           ; af
    or b                            ; b0
    or c                            ; b1
    or d                            ; b2
    or e                            ; b3
    or h                            ; b4
    or l                            ; b5
    or [hl]                         ; b6
    or a                            ; b7
    cp b                            ; b8
    cp c                            ; b9
    cp d                            ; ba
    cp e                            ; bb
    cp h                            ; bc
    cp l                            ; bd
    cp [hl]                         ; be
    cp a                            ; bf

    ; C0-FF
    ret nz                          ; c0
    pop bc                          ; c1
    jp nz, _start                   ; c2
    jp _start                       ; c3
    call nz, _start                 ; c4
    push bc                         ; c5
    add a, 0x42                     ; c6
    rst 0                           ; c7
    ret z                           ; c8
    ret                             ; c9
    jp z, _start                    ; ca
    call z, _start                  ; cc
    call _start                     ; cd
    adc a, 0x42                     ; ce
    rst 8                           ; cf

    ret nc                          ; d0
    pop de                          ; d1
    jp nc, _start                   ; d2
    out [0x42], a                   ; d3
    call nc, _start                 ; d4
    push de                         ; d5
    sub 0x42                        ; d6
    rst 0x10                        ; d7
    ret c                           ; d8
    exx                             ; d9
    jp c, _start                    ; da
    in a, [0x42]                    ; db
    call c, _start                  ; dc
    sbc a, 0x42                     ; de
    rst 0x18                        ; df

    ret po                          ; e0
    pop hl                          ; e1
    jp po, _start                   ; e2
    ex [sp], hl                     ; e3
    call po, _start                 ; e4
    push hl                         ; e5
    and 0x0F                        ; e6
    rst 0x20                        ; e7
    ret pe                          ; e8
    jp pe, _start                   ; ea
    ex de, hl                       ; eb
    call pe, _start                 ; ec
    xor 0xFF                        ; ee
    rst 0x28                        ; ef

    ret p                           ; f0
    pop af                          ; f1
    jp p, _start                    ; f2
    di                              ; f3
    call p, _start                  ; f4
    push af                         ; f5
    or 0xF0                         ; f6
    rst 0x30                        ; f7
    ret m                           ; f8
    ld sp, hl                       ; f9
    jp m, _start                    ; fa
    ei                              ; fb
    call m, _start                  ; fc
    cp 0x42                         ; fe
    rst 0x38                        ; ff

    ; === CB PREFIXED ===
    rlc b                           ; cb 00
    rlc c                           ; cb 01
    rlc d                           ; cb 02
    rlc e                           ; cb 03
    rlc h                           ; cb 04
    rlc l                           ; cb 05
    rlc [hl]                        ; cb 06
    rlc a                           ; cb 07
    rrc b                           ; cb 08
    rrc c                           ; cb 09
    rrc a                           ; cb 0f
    rl b                            ; cb 10
    rr b                            ; cb 18
    sla b                           ; cb 20
    sra b                           ; cb 28
    sll b                           ; cb 30
    srl b                           ; cb 38

    bit 0, b                        ; cb 40
    bit 1, c                        ; cb 49
    bit 2, d                        ; cb 52
    bit 3, e                        ; cb 5b
    bit 4, h                        ; cb 64
    bit 5, l                        ; cb 6d
    bit 6, [hl]                     ; cb 76
    bit 7, a                        ; cb 7f

    res 0, b                        ; cb 80
    res 1, c                        ; cb 89
    res 2, d                        ; cb 92
    res 3, e                        ; cb 9b
    res 4, h                        ; cb a4
    res 5, l                        ; cb ad
    res 6, [hl]                     ; cb b6
    res 7, a                        ; cb bf

    set 0, b                        ; cb c0
    set 1, c                        ; cb c9
    set 2, d                        ; cb d2
    set 3, e                        ; cb db
    set 4, h                        ; cb e4
    set 5, l                        ; cb ed
    set 6, [hl]                     ; cb f6
    set 7, a                        ; cb ff

    ; === ED PREFIXED ===
    ld bc, [0x1234]                 ; ed 4b
    ld [0x1234], bc                 ; ed 43
    ld de, [0x1234]                 ; ed 5b
    ld [0x1234], de                 ; ed 53
    ld hl, [0x1234]                 ; 2a (not ed)
    ld [0x1234], hl                 ; 22 (not ed)
    ld sp, [0x1234]                 ; ed 7b
    ld [0x1234], sp                 ; ed 73

    in b, [c]                       ; ed 40
    in c, [c]                       ; ed 48
    in d, [c]                       ; ed 50
    in e, [c]                       ; ed 58
    in h, [c]                       ; ed 60
    in l, [c]                       ; ed 68
    in a, [c]                       ; ed 78

    out [c], b                      ; ed 41
    out [c], c                      ; ed 49
    out [c], d                      ; ed 51
    out [c], e                      ; ed 59
    out [c], h                      ; ed 61
    out [c], l                      ; ed 69
    out [c], a                      ; ed 79

    sbc hl, bc                      ; ed 42
    sbc hl, de                      ; ed 52
    sbc hl, hl                      ; ed 62
    sbc hl, sp                      ; ed 72

    adc hl, bc                      ; ed 4a
    adc hl, de                      ; ed 5a
    adc hl, hl                      ; ed 6a
    adc hl, sp                      ; ed 7a

    neg                             ; ed 44
    retn                            ; ed 45
    reti                            ; ed 4d
    rrd                             ; ed 67
    rld                             ; ed 6f
    ldi                             ; ed a0
    ldir                            ; ed b0
    ldd                             ; ed a8
    lddr                            ; ed b8
    cpi                             ; ed a1
    cpir                            ; ed b1
    cpd                             ; ed a9
    cpdr                            ; ed b9
    ini                             ; ed a2
    inir                            ; ed b2
    ind                             ; ed aa
    indr                            ; ed ba
    outi                            ; ed a3
    otir                            ; ed b3
    outd                            ; ed ab
    otdr                            ; ed bb
    ld i, a                         ; ed 47
    ld a, i                         ; ed 57
    ld r, a                         ; ed 4f
    ld a, r                         ; ed 5f
    im 0                            ; ed 46 / ed 4e / ed 66 / ed 6e
    im 1                            ; ed 56 / ed 76
    im 2                            ; ed 5e / ed 7e

    ; === DD/FD (IX/IY) INSTRUCTIONS ===
    ld ix, 0x1234                   ; dd 21
    ld iy, 0x1234                   ; fd 21
    ld [0x5678], ix                 ; dd 22
    ld [0x5678], iy                 ; fd 22
    ld ix, [0x5678]                 ; dd 2a
    ld iy, [0x5678]                 ; fd 2a

    add ix, bc                      ; dd 09
    add ix, de                      ; dd 19
    add ix, ix                      ; dd 29
    add ix, sp                      ; dd 39
    add iy, bc                      ; fd 09
    add iy, de                      ; fd 19
    add iy, iy                      ; fd 29
    add iy, sp                      ; fd 39

    inc ix                          ; dd 23
    inc iy                          ; fd 23
    dec ix                          ; dd 2b
    dec iy                          ; fd 2b

    push ix                         ; dd e5
    push iy                         ; fd e5
    pop ix                          ; dd e1
    pop iy                          ; fd e1

    ld sp, ix                       ; dd f9
    ld sp, iy                       ; fd f9

    jp [ix]                         ; dd e9
    jp [iy]                         ; fd e9

    ex [sp], ix                     ; should be dd e3
    ex [sp], iy                     ; should be fd e3

    ; IX/IY indexed addressing
    ld a, [ix+5]                    ; dd 7e 05
    ld [ix+5], a                    ; dd 77 05
    ld b, [ix+5]                    ; dd 46 05
    ld [ix+5], b                    ; dd 70 05
    inc [ix+5]                      ; dd 34 05
    dec [ix+5]                      ; dd 35 05
    ld [ix+5], 0x42                 ; dd 36 05

    ld a, [iy-3]                    ; fd 7e fd
    ld [iy-3], a                    ; fd 77 fd
    inc [iy-3]                      ; fd 34 fd
    dec [iy-3]                      ; fd 35 fd

    ; IX/IY ALU
    add a, [ix+5]                   ; dd 86 05
    adc a, [ix+5]                   ; dd 8e 05
    sub [ix+5]                      ; dd 96 05
    sbc a, [ix+5]                   ; dd 9e 05
    and [ix+5]                      ; dd a6 05
    xor [ix+5]                      ; dd ae 05
    or [ix+5]                       ; dd b6 05
    cp [ix+5]                       ; dd be 05

    ; IX/IY CB bit ops
    rlc [ix+5]                      ; dd cb 05 06
    rrc [ix+5]                      ; dd cb 05 0e
    rl [ix+5]                       ; dd cb 05 16
    rr [ix+5]                       ; dd cb 05 1e
    sla [ix+5]                      ; dd cb 05 26
    sra [ix+5]                      ; dd cb 05 2e
    sll [ix+5]                      ; dd cb 05 36
    srl [ix+5]                      ; dd cb 05 3e

    bit 0, [ix+5]                   ; dd cb 05 46
    res 1, [ix+5]                   ; dd cb 05 8e
    set 2, [ix+5]                   ; dd cb 05 d6

    ; undocumented OUT (C),0
    out [c], 0                      ; ed 71

    ; undocumented IN f,(c) - if supported
    ; in f, [c]                     ; ed 70
