section text
global _start
_start:
    adc hl,hl 

dw lbl1, lbl2, lbl3
lbl1: db "oi1"
lbl2: db "oi2"
lbl3: db "oi3"