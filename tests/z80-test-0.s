section text
global _main
_main:
    ld iy, 2
    add iy, sp
    ld ix, 2
    add ix, sp
    

dw lbl1, lbl2, lbl3
lbl1: db "oi1"
lbl2: db "oi2"
lbl3: db "oi3"