;; 8080 assembler test — basic instructions
section text
global _start
_start:
    ; 8-bit load
    mvi a, 0x42
    mvi b, 0x10
    mvi c, 0x20
    mvi d, 0x30
    mvi e, 0x40
    mvi h, 0x50
    mvi l, 0x60
    mov b, a
    mov c, b
    mov d, c
    mov e, d
    mov h, e
    mov l, h
    mov a, l

    ; 16-bit load
    lxi h, 0x1234
    lxi d, 0x5678
    lxi b, 0x9ABC
    lxi sp, 0xFFFE
    sphl

    ; push / pop
    push h
    push d
    push b
    pop b
    pop d
    pop h

    ; arithmetic
    add a
    adc a
    sub a
    sbb a
    ana a
    ora a
    xra a
    cmp a
    inr a
    dcr a
    add b
    adc b
    sub b
    sbb b
    ana b
    ora b
    xra b
    cmp b

    ; 16-bit arithmetic
    dad h
    dad d
    dad b
    dad sp
    inx h
    inx d
    inx b
    dcx h
    dcx d
    dcx b

    ; exchange / rotate
    xchg
    rlc
    rrc
    ral
    rar

    ; accumulator special
    daa
    cma
    stc
    cmc

    ; jump / call / ret
    jmp .L1
    jz .L1
    jnz .L1
    jc .L1
    jnc .L1
    jm .L1
    jp .L1
    jpe .L1
    jpo .L1
    call .L1
    ret

.L1:
    ; conditional call/ret
    cz .L1
    cnz .L1
    rz
    rnz

    ; memory
    sta 0x1000
    lda 0x1000
    stax b
    stax d
    ldax b
    ldax d
    mov a, m
    mov m, a

    ; I/O
    in 0x10
    out 0x10

    ; interrupts
    ei
    di
    hlt
    nop

    ret

section data
var: dw 0
