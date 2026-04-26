bdos equ 5
    .open_fcb equ 15
    .close_fcb equ 16
    .read_fcb equ 20
    .write_char equ 2
    .print equ 9
    .exit equ 0
var_fcb equ 0x5c
var_buffer equ 0x80

section text
global _main
_main:
    or [ix+0]
    ld de, var_fcb
    ld c, bdos.open_fcb
    call bdos

    cp 255
    jp z, .error

    .read_loop:
        ld de, var_fcb
        ld c, bdos.read_fcb
        call bdos

        cp 0
        jr nz, .exit

        ld hl, var_buffer
        ld b, 128
        .print_loop:
            ;ld b,b
            ld a, [hl]
            cp 0x1a
            jr z, .exit
            cp 0
            jr z, .exit
            
            push bc
            push hl

            ld e, a
            ld c, bdos.write_char
            call bdos

            pop hl
            pop bc

            inc hl
            djnz .print_loop
        
        jr .read_loop

    .error:
        ld de, msg_error
        ld c, bdos.print
        call bdos
        ld a, '$'
        ld [var_fcb + 13], a
        ld de, var_fcb + 1
        ld c, bdos.print
        call bdos
        ld c, 0x62
        ld b, 1
        call bdos
        ld c, 0
        call bdos
    .exit:
        ld c, bdos.close_fcb
        ld de, var_fcb
        call bdos

        ; TEST VDP
        ld b,b
        di
        ld a, 0
        out [0x99], a
        ld a, 0x40
        out [0x99], a
        ld hl, msg
        ld bc, 0x0498
        otir
        ei

        ld c, 0
        call bdos

section data
msg: db "TEST"
msg_error: db "error: file not found: $"