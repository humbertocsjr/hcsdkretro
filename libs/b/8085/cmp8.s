section text
    global cmpe8
cmpe8:
    cmp e
    sbb a
    ret

    global cmpne8
cmpne8:
    cmp e
    sbb a
    cma
    ret

    global cmpa8
cmpa8:
    cmp e
    cmc
    sbb a
    ani 0xfe
    ret

    global cmpb8
cmpb8:
    cmp e
    sbb a
    ret

    global cmpae8
cmpae8:
    cmp e
    cmc
    sbb a
    ret

    global cmpbe8
cmpbe8:
    cmp e
    sbb a
    cma
    ret
