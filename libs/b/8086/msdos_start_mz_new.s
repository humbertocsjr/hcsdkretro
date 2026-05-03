; MS-DOS 8086 MZ EXE _start — Independent segments (32-bit far pointers)
; CS = TEXT, DS = ES = DATA, SS = BSS
; Stack at end of BSS, SP = __stack_top__
; Segment values computed at runtime as CS + __data_seg_delta__ / CS + __bss_seg_delta__
global _start
extern main

section text
_start:
    ; Save PSP segment (DS=ES=PSP on entry)
    push ds

    ; DATA segment = CS + __data_seg_delta__
    mov ax, cs
    add ax, __data_seg_delta__
    mov ds, ax
    mov es, ax

    ; BSS segment = CS + __bss_seg_delta__
    mov ax, cs
    add ax, __bss_seg_delta__
    cli
    mov ss, ax
    mov sp, __stack_top__
    sti

    ; Clear BSS (SS:0 .. SS:bss_size-1)
    mov cx, __bss_size__
    jcxz .bss_clear_done
    xor di, di
    xor al, al
    cld
    push ds
    push es
    mov ax, ss
    mov es, ax
    rep stosb
    pop es
    pop ds
.bss_clear_done:

    ; Parse command line from PSP
    pop ds              ; DS = saved PSP segment
    xor cx, cx
    mov cl, [0x0080]    ; command line length byte
    jcxz .call_main     ; no args

    ; Copy command line to _argv_buf in DATA segment
    push ds             ; save PSP
    mov si, 0x0081
    push es             ; save ES=DATA
    push cx             ; save count

    ; ES = DATA, DS = PSP
    mov ax, cs
    add ax, __data_seg_delta__
    mov es, ax
    mov di, _argv_buf
    cld
    rep movsb           ; copy DS:SI(PSP) → ES:DI(DATA)
    xor al, al
    stosb               ; null terminate

    pop cx              ; CX = byte count
    pop es              ; ES = DATA
    pop ds              ; DS = PSP

    ; Now parse arguments from _argv_buf in DATA
    push ds             ; save PSP for later
    mov ax, cs
    add ax, __data_seg_delta__
    mov ds, ax          ; DS = DATA segment
    mov si, _argv_buf
    mov bx, _argv
    xor ch, ch          ; CH = argc

.skip_spaces:
    lodsb               ; AL = [DS:SI], SI++
    cmp al, ' '
    je .skip_spaces
    cmp al, 13          ; CR
    je .parse_done
    or al, al
    jz .parse_done
    dec si              ; unget

    ; Store far pointer to this arg
    mov [bx], si        ; offset within DATA segment
    mov [bx+2], ds     ; DATA segment
    add bx, 4
    inc ch

.find_end:
    lodsb
    cmp al, ' '
    je .end_token
    cmp al, 13
    je .end_token
    or al, al
    jz .end_token
    jmp .find_end

.end_token:
    xor al, al
    mov [si-1], al   ; null-terminate
    jmp .skip_spaces

.parse_done:
    ; Null-terminate argv array
    xor ax, ax
    mov [bx], ax
    mov [bx+2], ax
    mov al, ch           ; argc in AL

    pop ds               ; DS = PSP
    jmp .do_call

.call_main:
    xor ax, ax           ; argc = 0
    ; Fall through to null-terminate argv

.do_call:
    ; Restore DS = ES = DATA
    push ax              ; save argc
    mov ax, cs
    add ax, __data_seg_delta__
    mov ds, ax
    mov es, ax
    pop ax               ; restore argc

    ; main(argc, _argv)
    xor dx, dx           ; argc high = 0
    push dx              ; argc high
    push ax              ; argc low
    mov cx, cs
    add cx, __data_seg_delta__
    push cx              ; segment of argv
    mov cx, _argv        ; offset of argv
    push cx
    call main
    add sp, 8

    ; Exit with return code in AX (low 8 bits)
    mov ah, 0x4C
    int 0x21

section data
_argv:    rb 128
_argv_buf: rb 256
