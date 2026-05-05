; 32-bit I/O library for MZ EXE (far pointer model)
; DS=DATA on entry (preserved by caller convention)

; putchar(c) - print character
global putchar
section text
putchar:
    push bp
    mov bp, sp
    mov dl, [bp+4]        ; low byte of 32-bit arg
    mov ah, 2
    int 0x21
    pop bp
    ret

; getchar() - read character, return 32-bit (zero-extended in DX:AX)
global getchar
section text
getchar:
    push bp
    mov bp, sp
    mov ah, 1
    int 0x21
    xor ah, ah
    xor dx, dx             ; return value in DX:AX with DX=0
    pop bp
    ret

; puts(str) - print string from far pointer
; Parameter: [bp+4] = far pointer (offset), [bp+6] = segment
global puts
section text
puts:
    push bp
    mov bp, sp
    push ds                ; save DS
    mov dx, [bp+4]         ; offset from far pointer
    mov ds, [bp+6]         ; segment from far pointer
    mov ah, 9
    int 0x21
    pop ds                 ; restore DS
    pop bp
    ret

; exit(code) - terminate program
global exit
section text
exit:
    push bp
    mov bp, sp
    mov al, [bp+4]         ; low byte of 32-bit exit code
    mov ah, 0x4C
    int 0x21
    pop bp
    ret
