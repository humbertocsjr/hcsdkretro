; CP/M 8085 _start — clears BSS, parses command line, calls main(argc, argv)
; Based on Z80 version
global _start
extern main

section text
_start:
    ; Initial stack at end of BSS + stack reserve (provided by linker)
    lxi h, __stack_top__
    sphl

    ; Clear BSS
    lxi h, __bss_start__
    lxi b, __bss_size__
    mov a, b
    ora c
    jz .bss_done
.bss_loop:
    mvi m, 0
    inx h
    dcx b
    mov a, b
    ora c
    jnz .bss_loop
.bss_done:

    ; Parse command line at 0080h: byte 0 = length
    lxi h, _argv_buf
    shld _buf_ptr
    lxi h, _argv
    shld _argv_ptr
    lda 0x0080
    mov c, a
    ora a
    jz .done

    mov b, a
    lxi h, 0x0081

.skip:
    mov a, m
    cpi ' '
    jnz .token
    inx h
    dcr b
    jnz .skip
    jmp .done

.token:
    push h
    lhld _argv_ptr
    xchg
    pop h
    xchg
    mov m, e
    inx h
    mov m, d
    inx h
    shld _argv_ptr
    inr c

    lhld _buf_ptr
.cp:
    xchg
    mov a, m
    xchg
    cpi ' '
    jz .endtok
    ora a
    jz .eol
    mov m, a
    inx h
    inx d
    dcr b
    jnz .cp
    jmp .eol

.endtok:
    mvi m, 0
    inx h
    shld _buf_ptr
    inx d
    dcr b
    jnz .skip

.eol:
    mvi m, 0
    inx h
    shld _buf_ptr

.done:
    lhld _argv_ptr
    mvi m, 0
    inx h
    mvi m, 0

    ; main(argc, argv)
    lxi h, _argv
    push h
    lxi h, 0
    mov l, c
    push h
    call main

    ; Exit via BDOS 0
    mvi c, 0
    call 5

section data
_argv:     ds 64*2
_argv_buf: ds 256
_buf_ptr:  dw 0
_argv_ptr: dw 0
