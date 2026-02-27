global strlen
strlen:
    push psw            ; salva af
    push de             ; salva de
    push bc             ; salva bc
    sub a               ; zera a
    mov b, a            ; zera b
    mov c, a            ; zera c
    .loop:
        mov a, [hl]     ; le [hl] para a
        cpi 0           ; confere se a = 0
        jz .end         ; se zero encerra
        inr c           ; incrementa c
        jnz .advance    ; se ultrapassou 255
        inr b           ; incrementa b
        .advance:       ; continua se c < 255
        inx hl          ; incrementa hl
        jmp .loop       ; va pro proximo caractere
    .end:
    mov l, c            ; define hl = bc
    mov h, b            ;
    pop bc              ; restaura bc
    pop de              ; restaura de
    pop psw             ; restaura af
    ret