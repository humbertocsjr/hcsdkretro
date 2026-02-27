
add a, 0x12
add a, b
add b
add a, [hl]
add a, [ix+3]
add a, [iy+3]
add hl, bc
sub b
sbc hl, bc
sub a, 1
sbc a, 1
cp a, 1
djnz lbl
lbl:
jr nz, lbl
jr z, lbl
jr c, lbl
jr nc, lbl
jr lbl
inc [hl]
inc [ix+3]
inc [iy+3]
inc hl
inc ix
inc bc
inc a
inc h
inc l
dec [hl]
dec [ix+3]
dec [iy+3]
dec hl
dec ix
dec bc
dec a
dec h
dec l
ret
ret nz
ret nc
push bc
push af
pop bc
pop af
push ix
pop iy
jp lbl
jp nz, lbl
jp nc, lbl
jp p, lbl
rst 0x00
rst 0x38
ex af, af
ex de, hl
ex hl, de
ex [sp], hl
ex hl, [sp]
ld a, b
ld c, e
ld h, l
ld a, [hl]
ld [hl], a
ld a, 55
ld d, 0x12
ld [hl], 0x12
ld [ix+5], 0x12
ld [ix+5], d
ld e, [iy+4]
ld e, [hl]
ld [bc], a
ld [de], a
ld a, [bc]
ld a, [de]
ld hl, 0x1234
ld bc, 0x1234
ld sp, 0x1234
ld [0x1234], hl
ld [0x1234], ix
ld [0x1234], iy
ld [0x1234], a
ld hl, [0x1234]
ld ix, [0x1234]
ld iy, [0x1234]
ld a, [0x1234]
ld sp, ix
ld sp, hl
ld sp, iy
ld [0x1234], bc
ld [0x1234], hl
ld [0x1234], sp
ld bc, [0x1234]
ld hl, [0x1234]
ld sp, [0x1234 + (5*16)]
ld a,i
ld i,a
ld a,r
ld r,a
bit 5, b
res 5, b
set 5, b
nop