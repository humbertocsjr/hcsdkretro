;; 8085 assembler test — basic instructions
section text
global _start
_start:
    mvi a, 0x42
    mvi b, 0x10
    mov c, b
    lxi h, 0x1234
    push h
    pop d
    dad d
    inx h
    dcx d
    add b
    sub c
    ana d
    ora e
    xra a
    cmp b
    jmp .L1
    jz .L1
    jnz .L1
    call .L1
    ret
.L1:
    rim
    sim
    rar
    nop
    ret
