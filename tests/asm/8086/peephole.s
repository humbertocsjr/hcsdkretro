;; 8086 assembler test - peephole-generated patterns
section text
global _start
_start:
    push bp
    mov bp, sp

    ; mov word [bp-N], imm  (peephole store)
    mov word [bp-2], 1
    mov word [bp-4], 42

    ; mov ax, [bp-N]  (peephole load)
    mov ax, [bp-2]
    mov bx, [bp-4]

    ; mov ax, [label]  (peephole global deref)
    mov ax, [g_var]

    ; mov [bx], ax  (register-indirect store)
    lea ax, [bp-2]
    mov bx, ax
    mov word [bx], 0

    ; push/pop → mov
    push ax
    pop bx

    ; immediate to accumulator
    mov ax, 0x1234
    mov al, 0x42

    ; string instructions
    rep movsb
    rep stosb

    ; segment prefixes
    es mov ax, [bx]
    ss mov [bp-2], ax

    pop bp
    ret

section data
g_var: dw 0
