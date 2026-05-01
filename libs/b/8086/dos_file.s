; MS-DOS File I/O Runtime (minimal stub)
; Full implementation requires INT 21h file operations
global fopen
fopen:
    push bp
    mov bp, sp
    mov ax, -1
    pop bp
    ret

global fclose
fclose:
    push bp
    mov bp, sp
    mov ax, -1
    pop bp
    ret

global fread
fread:
    push bp
    mov bp, sp
    mov ax, -1
    pop bp
    ret

global fwrite
fwrite:
    push bp
    mov bp, sp
    mov ax, -1
    pop bp
    ret

global fseek
fseek:
    push bp
    mov bp, sp
    mov ax, -1
    pop bp
    ret

global ftell
ftell:
    push bp
    mov bp, sp
    mov ax, -1
    pop bp
    ret

global feof
feof:
    push bp
    mov bp, sp
    mov ax, 0
    pop bp
    ret

global fdelete
fdelete:
    push bp
    mov bp, sp
    mov ax, -1
    pop bp
    ret

global frename
frename:
    push bp
    mov bp, sp
    mov ax, -1
    pop bp
    ret

global fgetc
fgetc:
    push bp
    mov bp, sp
    mov ax, -1
    pop bp
    ret

global fputc
fputc:
    push bp
    mov bp, sp
    mov ax, -1
    pop bp
    ret
