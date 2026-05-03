; MS-DOS 8086 _start — clears BSS, parses command line, calls main(argc, argv)
global _start
extern main

section text
_start:
    mov sp, __stack_top__

    ; Clear BSS
    mov cx, __bss_size__
    jcxz .bss_done
    mov di, __bss_start__
    xor al, al
    cld
.rep:
    stosb
    loop .rep
.bss_done:

    ; Parse command line at DS:0080h
    mov si, 0x0081
    xor cx, cx
    mov cl, [0x0080]
    jcxz .done

    mov di, _argv_buf
    mov bx, _argv
    xor ch, ch

.skip:
    lodsb
    cmp al, ' '
    jne .token
    loop .skip
    jmp .done

.token:
    dec si
    mov [bx], di
    add bx, 2
    inc ch

.cp:
    lodsb
    cmp al, ' '
    je .endtok
    or al, al
    jz .eol
    stosb
    loop .cp
    jmp .eol

.endtok:
    xor al, al
    stosb
    loop .skip

.eol:
    xor al, al
    stosb

.done:
    xor ax, ax
    mov [bx], ax

    ; main(argc, argv)
    mov ax, _argv
    push ax
    xor ax, ax
    mov al, ch
    push ax
    call main

    ; Exit with return code
    mov ah, 0x4C
    int 0x21

section data
_argv:
    rb 128     ; 64 word-sized pointers
_argv_buf:
    rb 256     ; argument string storage
