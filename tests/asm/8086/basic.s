;; 8086 assembler test — basic instructions
section text
global _start
_start:
    ; mov
    mov ax, 0x1234
    mov bx, ax
    mov cx, 0x10
    mov dx, cx
    mov si, dx
    mov di, si
    mov bp, sp

    ; push / pop
    push ax
    push bx
    pop bx
    pop ax

    ; arithmetic
    add ax, bx
    sub ax, 1
    adc ax, 0
    sbb ax, cx
    and ax, 0x0F
    or  ax, 0xF0
    xor ax, dx
    cmp ax, 0x42
    inc ax
    dec bx
    mul bx
    div cx

    ; shifts
    shl ax, 1
    shr bx, 1
    rol cx, 1
    ror dx, 1

    ; exchange
    xchg ax, bx

    ; lea
    lea ax, [bp-2]
    lea ax, [bp+4]

    ; memory
    mov word [bp-2], 0
    mov [bx], ax
    mov ax, [bp+4]
    mov ax, [label]

    ; jumps
    jmp .L1
    je  .L1
    jne .L1
    jl  .L1
    jg  .L1
    jle .L1
    jge .L1
    je  .L2
    jmp .L1

.L1:
    ; conditional set (via jumps)
    cmp ax, 0
    je .L2
    mov ax, 0
    jmp .L3
.L2:
    mov ax, 1
.L3:

    ; call / ret
    call .L4
    ret
.L4:
    ret

section data
label: dw 0
