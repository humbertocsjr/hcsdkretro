; MS-DOS 8086 MZ EXE _start — small model (CS prefixed for lib)
; DS,ES at entry = PSP segment
; CS = code segment, must compute DS/ES/SS from CS
; DS = SS = data+stack segment (B compiler assumes DS=SS for [bx] access)
global _start
extern main

section data
_argv:
    rb 128
_argv_buf:
    rb 256

section text
_start:
    ; Save PSP segment (DS at entry points to PSP)
    push ds
    push es

    ; Set DS = ES = data+stack segment
    mov ax, cs
    add ax, __data_seg_delta__
    mov ds, ax
    mov es, ax

    ; Set SS = DS (same segment — B compiler uses [bx] for stack ptrs)
    cli
    mov ss, ax
    mov sp, __stack_top__
    sti

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

    ; Parse command line via saved PSP segment
    ; Stack now contains: [saved ES] [saved DS] (PSP segments)
    pop es      ; ES = PSP segment (was saved as DS from entry)
    pop ds      ; DS = PSP segment

    mov si, 0x0081
    xor cx, cx
    mov cl, [0x0080]
    jcxz .done

    ; Temporarily switch to data segment for writing argv
    push es
    mov ax, cs
    add ax, __data_seg_delta__
    mov ds, ax
    mov es, ax

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
    ; Ensure DS = data segment for main
    mov ax, cs
    add ax, __data_seg_delta__
    mov ds, ax
    mov es, ax

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
