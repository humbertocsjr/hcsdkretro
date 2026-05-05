section text
    global shl8
shl8:
    mov b, e
    mov c, a
    mov a, b
    ora a
    mov a, c
    rz
.l:
    add a
    dcr b
    jnz .l
    ret

    global shl16
shl16:
    mov a, e
    ora a
    rz
    mov b, e
.l:
    dad h
    dcr b
    jnz .l
    ret

    global shr8
shr8:
    mov b, e
    mov c, a
    mov a, b
    ora a
    mov a, c
    rz
.l:
    rar
    dcr b
    jnz .l
    ret

    global shr16
shr16:
    mov a, e
    ora a
    rz
    mov b, e
.l:
    mov a, h
    rar
    mov h, a
    mov a, l
    rar
    mov l, a
    dcr b
    jnz .l
    ret
