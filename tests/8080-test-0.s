section text
mov a, b
global _start
_start:
lbl1:


times 7 nop
lbl2:
nop
jmp lbl2
mvi a, 4
lbl3:
lhld lbl3 + 34 - 5
call strlen
mvi a, 32

section bss
var: resb 1
var2: resw 1