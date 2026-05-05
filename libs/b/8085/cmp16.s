section text
    global cmpe16
cmpe16:
    mov a, l
    sub e
    mov a, h
    sbb d
    lxi h, 0
    rnz
    dcx h
    ret

    global cmpne16
cmpne16:
    mov a, l
    sub e
    mov a, h
    sbb d
    lxi h, 0
    rz
    dcx h
    ret

    global cmpa16
cmpa16:
    mov a, l
    sub e
    mov a, h
    sbb d
    lxi h, 0
    rc
    rz
    dcx h
    ret

    global cmpb16
cmpb16:
    mov a, l
    sub e
    mov a, h
    sbb d
    lxi h, 0
    rnc
    dcx h
    ret

    global cmpae16
cmpae16:
    mov a, l
    sub e
    mov a, h
    sbb d
    lxi h, 0
    rc
    dcx h
    ret

    global cmpbe16
cmpbe16:
    mov a, l
    sub e
    mov a, h
    sbb d
    lxi h, 0
    rc
    rz
    dcx h
    ret
