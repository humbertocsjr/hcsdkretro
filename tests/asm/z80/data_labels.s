;; Z80 assembler test - sections, data, labels, expressions
section data
var_label: dw 0x1234
arr_label: ds 16

section text
global _start
_start:
    ; label references
    ld hl, var_label
    ld de, arr_label

    ; expressions
    ld hl, var_label + 2
    ld de, arr_label - var_label
    ld bc, 100 * 4

    ; current position
    jr $

    ; local labels
.local_loop:
    jr .local_loop

    ret
