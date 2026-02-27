
section data
msg: db "TEST",13,10,"$"


section text
global _start
_start:
    mov ah, 0x9
    mov dx, msg
    int 0x21
    int 0x20 ; old dos exit
