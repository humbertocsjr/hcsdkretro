section text
    global __div8
__div8:
    global __divs8
__divs8:
    mov b, a
    xra e
    push psw
    mov a, b
    ani 0x80
    jz .a_pos
    mov a, b
    cma
    inr a
    mov b, a
.a_pos:
    mov a, e
    ani 0x80
    jz .e_pos
    mov a, e
    cma
    inr a
.e_pos:
    mov e, a
    mov a, b
    call __divu8
    pop psw
    jp .done
    cma
    inr a
.done:
    ret

    global __divu8
__divu8:
    mvi b, 0
    mov c, a
    mvi a, 8
.l:
    mov a, c
    add a
    mov c, a
    mov a, b
    ral
    mov b, a
    mov a, c
    sub e
    jc .skip
    mov c, a
    inr b
.skip:
    dcr a
    jnz .l
    mov a, b
    mov e, c
    ret

    global __mod8
__mod8:
    call __divs8
    mov a, e
    ret

    global __modu8
__modu8:
    call __divu8
    mov a, e
    ret
