section text
global _main
_main:
    ld hl, msg
    ld ix, msg
    call print
    ld hl, msg_ask
    call print
    ld de, name_buffer
    ld c, 0xa
    call 5
    ld hl, msg_hello
    call print
    ld b,b ; breakpoint
    ld ix, name_buffer
    ld l, [ix+1]
    ld h, 0
    ld de, 2
    add hl, de
    ld de, name_buffer
    add hl, de
    xor a
    ld [hl], a
    ld hl, name_buffer
    ld de, 2
    add hl, de
    call print
    ret
print:
    .loop:
        ld a, [hl]
        cp 0
        jr z, .end
        ld e, a
        ld c, 2
        call 5
        inc hl
        jr .loop
    .end:
    ret

section data
msg: db "Hellorld!",0
msg_ask: db 13,10,"Name: ",0
msg_hello: db 13,10,"Hello ",0
name_buffer: db 15,0
             resb 16