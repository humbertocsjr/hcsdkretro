section text
global _main
_main:
    ld hl, msg
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
    