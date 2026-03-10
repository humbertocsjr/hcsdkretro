bdos equ 5
    .print equ 9
    .readline equ 10

section data

    msg_prompt: db "= $"
    msg_crlf: db 13,10,"$"
    msg_error: db "Syntax error",13,10,"$"

    cmd_exit: db "exit",0

    pointer: dw 0

    in_buffer: 
        db 80, 0
        ds 80

    num_buffer:
        ds 8
    out_buffer:
        ds 10

section text

    global _start
    _start:
        ld de, msg_prompt
        call print_string

        ld de, in_buffer
        ld c, bdos.readline
        call bdos

        ld hl, in_buffer+2
        ld [pointer], hl
        call skip_spaces

        ld de, cmd_exit
        call cmp_string
        jr z, .exit

        call parse_expr
        jp c, syntax_error

        call skip_spaces
        call peek
        cp 0xd
        jp nz, syntax_error

        push hl

        ld de, msg_crlf
        call print_string

        pop hl

        call print_number

        ld de, msg_crlf
        call print_string

        jp _start
        .exit:
            ret
    
    ; PARSER

    parse_expr:
        call parse_term
        jr c, parse_error
        .loop:
            call skip_spaces
            call peek
            cp '+'
            jr z, .parse_add
            cp '-'
            jr z, .parse_sub
            ret
        .parse_add:
            call next
            push hl
            call parse_term
            jr c, parse_error_pop
            pop de
            add hl, de
            jr .loop
        .parse_sub:
            call next
            push hl
            call parse_term
            jr c, parse_error_pop
            pop de
            or a
            sbc hl, de
            ex de, hl
            ld h, d
            ld l, e
            jr .loop
    

    parse_error_pop:
        pop de
    parse_error:
        scf
        ret

    parse_term:
        call parse_unary
        jr c, parse_error
        .loop:
            call skip_spaces
            call peek

            cp '*'
            jr z, .parse_mul
            cp '/'
            jr z, .parse_div

            ret
        .parse_mul:
            call next
            push hl
            call parse_unary
            jr c, parse_error_pop
            pop de
            call mul16
            jr .loop
        .parse_div:
            call next
            push hl
            call parse_unary
            jr c, parse_error_pop
            pop de
            call div16
            jr .loop

    parse_unary:
        call skip_spaces
        call peek

        cp '+'
        jr z, .parse_pos
        cp '-'
        jr z, .parse_neg

        jp parse_primary

        .parse_pos:
            call next
            jp parse_primary
        .parse_neg:
            call next
            call parse_primary
            ret c
            ld de, 0
            or a
            sbc hl, de
            ret

    parse_primary:
        call skip_spaces
        call peek

        cp '('
        jr z, .parse_paren

        call parse_num
        .parse_paren:
            call next
            call parse_expr
            ret c
            
            call skip_spaces
            call peek

            cp ')'
            jr nz, parse_error

            call next
            ret

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
            jp z, parse_error
            or a
            ret
    
    peek:
        ld hl, [pointer]
        ld a, [hl]
        ret
    
    next:
        ld hl, [pointer]
        inc hl
        ld [pointer], hl
        ret
    
    skip_spaces:
        ld hl, [pointer]
        .loop:
            ld a, [hl]
            cp ' '
            jr nz, .end
            inc hl
            jr .loop
        .end:
        ld [pointer], hl
        ret
    
    ; MATH

    mul16:
        ld bc, 0
        .loop:
            ld a, e
            or d
            jr z, .end
            srl d
            rr e
            jr nc, .loop
            add hl, bc
            jr .loop
        .end:
        ret
    
    div16:
        ld bc, 0
        ld a, 16
        .loop:
            add hl, hl
            rl c
            rl b
            sbc hl, de
            jr c, .skip
            inc bc
            jr .next
            .skip:
                add hl, de
            .next:
            dec a
            jr nz, .loop
        ld h, b
        ld l, c
        ret

    ; PRINT

    print_number:
        ld de, num_buffer + 6
        ld b, 0

        .loop:
            push bc
            ld bc, 10
            call div16
            pop bc

            add a, '0'
            dec de
            ld [de], a
            inc b

            ld a, h
            or l
            jr nz, .loop

        ld h, d
        ld l, e
        ld de, out_buffer
        
        .copy:
            ld a, [hl]
            ld [de], a
            inc hl
            inc de
            djnz .copy

        ld a, '$'
        ld [de], a

        ld de, out_buffer
        call print_string
        ret
    
    print_string:
        ld c, bdos.print
        jp bdos
    
    cmp_string:
        push hl
        .loop:
            ld a, [de]
            or a
            jr z, .equal
            cp [hl]
            jr nz, .not_equal
            inc hl
            inc de
            jr .loop
        .equal:
            ld a, [hl]
            or a
            jr z, .ok
        .not_equal:
            pop hl
            or 1
            ret
        .ok:
            pop hl
            xor a
            ret

    syntax_error:
        ld de, msg_error
        call print_string
        jp _start