; CP/M Z80 _start — clears BSS, parses command line, calls main(argc, argv)
global _start
extern main

section text
_start:
    ; Initial stack at end of BSS
    ld hl, __bss_start__
    ld de, __bss_size__
    add hl, de
    ld sp, hl

    ; Clear BSS
    ld hl, __bss_start__
    ld de, __bss_size__
    ld a, d
    or e
    jr z, .bss_done
.bss_loop:
    xor a
    ld [hl], a
    inc hl
    dec de
    ld a, d
    or e
    jr nz, .bss_loop
.bss_done:

    ; Parse command line at 0080h: byte 0 = length
    ld hl, _argv_buf
    ld [_buf_ptr], hl
    ld hl, _argv
    ld [_argv_ptr], hl
    xor a
    ld c, 0
    ld a, [0x0080]
    or a
    jr z, .done

    ld b, a
    ld hl, 0x0081

.skip:
    ld a, [hl]
    cp ' '
    jr nz, .token
    inc hl
    djnz .skip
    jr .done

.token:
    push hl
    ld hl, [_argv_ptr]
    pop de
    ld [hl], e
    inc hl
    ld [hl], d
    inc hl
    ld [_argv_ptr], hl
    inc c

    ld hl, [_buf_ptr]
.cp:
    ld a, [de]
    cp ' '
    jr z, .endtok
    or a
    jr z, .eol
    ld [hl], a
    inc hl
    inc de
    djnz .cp
    jr .eol

.endtok:
    xor a
    ld [hl], a
    inc hl
    ld [_buf_ptr], hl
    inc de
    djnz .skip

.eol:
    xor a
    ld [hl], a
    inc hl
    ld [_buf_ptr], hl

.done:
    ld hl, [_argv_ptr]
    ld [hl], 0
    inc hl
    ld [hl], 0

    ; main(argc, argv)
    ld hl, _argv
    push hl
    ld hl, 0
    ld l, c
    push hl
    call main

    ; Exit: BDOS 0
    ld c, 0
    call 5

section data
_argv:     ds 64*2
_argv_buf: ds 256
_buf_ptr:  dw 0
_argv_ptr: dw 0
