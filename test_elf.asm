; Simple test program for ELF32+DWARF4 generation

%define MY_CONST 0x1000

    org 0x100

start:
    mov ax, MY_CONST     ; Load constant
    mov bx, 0x5000      ; Load address
    mov cx, 256         ; Load counter
    ret

end start
