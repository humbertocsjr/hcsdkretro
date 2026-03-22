section text
    cbw:
        ld l, a     ; L recebe o byte original
        add a, a    ; desloca bit 7 para o carry
        sbc a, a    ; A = 0xFF se carry=1, senão 0
        ld h, a     ; H = extensão (00 ou FF)
        ret