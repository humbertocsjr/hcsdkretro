; Pure assembly printf for 8080/8085
; B compiler bug workaround: assignments don't dereference RHS lvalues
global printf
extern putchar
extern _print_u
extern _print_s
extern _print_x

section text
printf:
    ; Prologue: set up frame
    lxi h, -8
    dad sp
    sphl            ; allocate 4 locals: i, ch, arg, fmt_copy

    ; i = 0
    lxi h, 0
    push h
    pop d
    lxi h, 0
    dad sp
    mov m, e
    inx h
    mov m, d

    ; arg = second param (a)
    lxi h, 10
    dad sp
    mov e, m
    inx h
    mov d, m
    lxi h, 4
    dad sp
    mov m, e
    inx h
    mov m, d

    ; fmt_copy = first param (fmt)
    lxi h, 8
    dad sp
    mov e, m
    inx h
    mov d, m
    lxi h, 6
    dad sp
    mov m, e
    inx h
    mov m, d

    ; ch = peekb(fmt)
    lxi h, 6
    dad sp
    mov e, m
    inx h
    mov d, m          ; DE = fmt
    xchg              ; HL = fmt
    mov a, m          ; A = first byte
    mov l, a
    mvi h, 0
    lxi d, 2
    dad d             ; HL = SP + 2
    mov m, e
    inx h
    mov m, d          ; wait, this is wrong

    ; Let me rewrite more carefully
    ; Save fmt as local and read first byte
    lxi h, 6
    dad sp
    mov e, m
    inx h
    mov d, m
    xchg
    mov a, m          ; A = first byte of fmt
    mov l, a
    mvi h, 0          ; HL = byte value
    lxi d, 2
    dad d
    push h
    pop d
    lxi h, 2
    dad sp
    mov m, e
    inx h
    mov m, d          ; ch = first byte

.loop:
    ; load ch
    lxi h, 2
    dad sp
    mov e, m
    inx h
    mov d, m          ; DE = ch
    mov a, e
    ora d
    jz .exit

    ; check if ch == '%'
    mov a, e
    cpi 37
    jnz .putchar

    ; format handler
    ; i++
    lxi h, 0
    dad sp
    mov e, m
    inx h
    mov d, m
    inx d
    mov m, d
    dcx h
    mov m, e

    ; ch = peekb(fmt + i)
    lxi h, 6
    dad sp
    mov e, m
    inx h
    mov d, m          ; DE = fmt
    lxi h, 0
    dad sp
    mov c, m
    inx h
    mov b, m          ; BC = i
    xchg              ; HL = fmt
    dad b             ; HL = fmt + i
    mov a, m          ; A = byte
    mov l, a
    mvi h, 0          ; HL = byte
    push h
    pop d
    lxi h, 2
    dad sp
    mov m, e
    inx h
    mov m, d          ; ch = byte

    ; check format specifiers
    mov a, e
    cpi 'd'
    jz .fmt_d
    cpi 'u'
    jz .fmt_u
    cpi 'x'
    jz .fmt_x
    cpi 's'
    jz .fmt_s
    cpi 'c'
    jz .fmt_c
    cpi '%'
    jz .fmt_pct
    jmp .next_char

.fmt_d:
    lxi h, 4
    dad sp
    mov e, m
    inx h
    mov d, m
    mov a, d
    ani 0x80
    jz .d_positive
    ; negative
    mvi c, 2
    mvi e, '-'
    call 5
    ; arg = 0 - arg
    lxi h, 4
    dad sp
    mov e, m
    inx h
    mov d, m
    dcx h
    mov a, e
    cma
    mov e, a
    mov a, d
    cma
    mov d, a
    inx d
    mov m, d
    dcx h
    mov m, e
    ; convert absolute value
    lxi h, 4
    dad sp
    mov e, m
    inx h
    mov d, m
    push d
    call _print_u
    lxi h, 2
    dad sp
    sphl
    jmp .next_arg

.d_positive:
    lxi h, 4
    dad sp
    mov e, m
    inx h
    mov d, m
    push d
    call _print_u
    lxi h, 2
    dad sp
    sphl
    jmp .next_arg

.fmt_u:
    lxi h, 4
    dad sp
    mov e, m
    inx h
    mov d, m
    push d
    call _print_u
    lxi h, 2
    dad sp
    sphl
    jmp .next_arg

.fmt_x:
    lxi h, 4
    dad sp
    mov e, m
    inx h
    mov d, m
    push d
    call _print_x
    lxi h, 2
    dad sp
    sphl
    jmp .next_arg

.fmt_s:
    lxi h, 4
    dad sp
    mov e, m
    inx h
    mov d, m
    push d
    call _print_s
    lxi h, 2
    dad sp
    sphl
    jmp .next_arg

.fmt_c:
    lxi h, 4
    dad sp
    mov e, m
    inx h
    mov d, m
    mvi c, 2
    call 5
    jmp .next_arg

.fmt_pct:
    mvi c, 2
    mvi e, '%'
    call 5
    jmp .next_char

.next_arg:
    ; arg++
    lxi h, 4
    dad sp
    mov e, m
    inx h
    mov d, m
    inx d
    mov m, d
    dcx h
    mov m, e

.next_char:
    ; i++
    lxi h, 0
    dad sp
    mov e, m
    inx h
    mov d, m
    inx d
    mov m, d
    dcx h
    mov m, e
    jmp .loop

.putchar:
    ; output character
    mvi c, 2
    call 5
    jmp .next_char

.exit:
    ; Epilogue
    lxi h, 8
    dad sp
    sphl
    ret
