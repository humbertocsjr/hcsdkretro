; ====================================================================
; z80 alu/flags ultimate tester - 140 testes (8-bit, 16-bit, bcd)
; formato: jp absoluto, hex 0x, global _main, aspas duplas, lowcase
; ====================================================================

bdos    equ 0x0005
print_c equ 0x09

        global _main

_main:
        jp _stage2

        ; --- bloco 1: add 8-bit (01-05) ---
        ld de, msg_t01 
        call print 
        ld a, 0x00 
        add a, 0x00
        jp nz, is_fail 
        jp c, is_fail 
        call is_pass

        ld de, msg_t02 
        call print 
        ld a, 0xff 
        add a, 0x01
        jp nz, is_fail 
        jp nc, is_fail 
        call is_pass

        ld de, msg_t03 
        call print 
        ld a, 0x7f 
        add a, 0x01
        jp p, is_fail 
        jp po, is_fail 
        call is_pass

        ld de, msg_t04 
        call print 
        ld a, 0x0f 
        add a, 0x01
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass

        ld de, msg_t05 
        call print 
        ld a, 0x80 
        add a, 0x80
        jp nz, is_fail 
        jp nc, is_fail 
        jp m, is_fail 
        call is_pass

        ; --- bloco 2: adc 8-bit (06-10) ---
        ld de, msg_t06 
        call print 
        scf 
        ld a, 0x00 
        adc a, 0x00
        cp 0x01 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t07 
        call print 
        scf 
        ld a, 0xff 
        adc a, 0x00
        jp nz, is_fail 
        jp nc, is_fail 
        call is_pass

        ld de, msg_t08 
        call print 
        scf 
        ld a, 0x7f 
        adc a, 0x00
        jp po, is_fail 
        jp p, is_fail 
        call is_pass

        ld de, msg_t09 
        call print 
        or a 
        ld a, 0x00 
        adc a, 0x00
        jp nz, is_fail 
        call is_pass

        ld de, msg_t10 
        call print 
        scf 
        ld a, 0x0f 
        adc a, 0x00
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        cp 0x10 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 3: sub 8-bit (11-15) ---
        ld de, msg_t11 
        call print 
        ld a, 0x00 
        sub 0x00
        jp nz, is_fail 
        jp c, is_fail 
        call is_pass

        ld de, msg_t12 
        call print 
        ld a, 0x00 
        sub 0x01
        jp p, is_fail 
        jp nc, is_fail 
        cp 0xff 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t13 
        call print 
        ld a, 0x7f 
        sub 0x80
        jp po, is_fail 
        cp 0xff 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t14 
        call print 
        ld a, 0x10 
        sub 0x01
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass

        ld de, msg_t15 
        call print 
        ld a, 0x55 
        sub a
        jp nz, is_fail 
        push af 
        pop hl 
        bit 1, l 
        jp z, is_fail 
        call is_pass

        ; --- bloco 4: sbc 8-bit (16-20) ---
        ld de, msg_t16 
        call print 
        scf 
        ld a, 0x00 
        sbc a, 0x00
        jp nc, is_fail 
        cp 0xff 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t17 
        call print 
        or a 
        ld a, 0x05 
        sbc a, 0x05
        jp nz, is_fail 
        jp c, is_fail 
        call is_pass

        ld de, msg_t18 
        call print 
        scf 
        ld a, 0xff 
        sbc a, 0x00
        cp 0xfe 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t19 
        call print 
        scf 
        ld a, 0x02 
        sbc a, 0x01
        jp nz, is_fail 
        call is_pass

        ld de, msg_t20 
        call print 
        scf 
        ld a, 0x55 
        sbc a, a
        jp nc, is_fail 
        cp 0xff 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 5: cp (21-25) ---
        ld de, msg_t21 
        call print 
        ld a, 0x00 
        cp 0x00
        jp nz, is_fail 
        jp c, is_fail 
        call is_pass

        ld de, msg_t22 
        call print 
        ld a, 0x00 
        cp 0x01
        jp nc, is_fail 
        jp p, is_fail 
        call is_pass

        ld de, msg_t23 
        call print 
        ld a, 0xfe 
        cp 0xff
        jp nc, is_fail 
        jp p, is_fail 
        call is_pass

        ld de, msg_t24 
        call print 
        ld a, 0x7f 
        cp 0x80
        jp p, is_fail 
        jp po, is_fail 
        call is_pass

        ld de, msg_t25 
        call print 
        ld a, 0x20 
        cp 0x10
        jp c, is_fail 
        call is_pass

        ; --- bloco 6: and (26-30) ---
        ld de, msg_t26 
        call print 
        ld a, 0xff 
        and 0x00
        jp nz, is_fail 
        jp c, is_fail 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass

        ld de, msg_t27 
        call print 
        ld a, 0x55 
        and a
        jp m, is_fail 
        jp po, is_fail 
        call is_pass

        ld de, msg_t28 
        call print 
        ld a, 0xaa 
        and 0x55
        jp nz, is_fail 
        jp po, is_fail 
        call is_pass

        ld de, msg_t29 
        call print 
        ld a, 0x80 
        and 0x80
        jp p, is_fail 
        jp c, is_fail 
        call is_pass

        ld de, msg_t30 
        call print 
        ld a, 0xff 
        and 0x0f
        cp 0x0f 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 7: or (31-35) ---
        ld de, msg_t31 
        call print 
        ld a, 0x00 
        or 0x00
        jp nz, is_fail 
        jp c, is_fail 
        call is_pass

        ld de, msg_t32 
        call print 
        ld a, 0x00 
        or 0xff
        jp p, is_fail 
        jp po, is_fail 
        call is_pass

        ld de, msg_t33 
        call print 
        ld a, 0x55 
        or a
        jp po, is_fail 
        call is_pass

        ld de, msg_t34 
        call print 
        ld a, 0x01 
        or 0x80
        jp p, is_fail 
        cp 0x81 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t35 
        call print 
        ld a, 0x7f 
        or 0x00
        jp c, is_fail 
        cp 0x7f 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 8: xor (36-40) ---
        ld de, msg_t36 
        call print 
        xor a
        jp nz, is_fail 
        jp c, is_fail 
        jp po, is_fail 
        call is_pass

        ld de, msg_t37 
        call print 
        ld a, 0xff 
        xor 0xff
        jp nz, is_fail 
        call is_pass

        ld de, msg_t38 
        call print 
        ld a, 0xaa 
        xor 0x55
        jp p, is_fail 
        cp 0xff 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t39 
        call print 
        ld a, 0x7f 
        xor 0x80
        cp 0xff 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t40 
        call print 
        ld a, 0x01 
        xor 0x01
        jp nz, is_fail 
        call is_pass

        ; --- bloco 9: inc 8-bit (41-45) ---
        ld de, msg_t41 
        call print 
        scf 
        ld a, 0xff 
        inc a
        jp nz, is_fail 
        jp nc, is_fail 
        call is_pass

        ld de, msg_t42 
        call print 
        ld a, 0x7f 
        inc a
        jp p, is_fail 
        jp po, is_fail 
        call is_pass

        ld de, msg_t43 
        call print 
        ld a, 0x0f 
        inc a
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass

        ld de, msg_t44 
        call print 
        ld a, 0x00 
        inc a
        jp z, is_fail 
        call is_pass

        ld de, msg_t45 
        call print 
        ld a, 0xfe 
        inc a
        jp p, is_fail 
        call is_pass

        ; --- bloco 10: dec 8-bit (46-50) ---
        ld de, msg_t46 
        call print 
        or a 
        ld a, 0x00 
        dec a
        jp p, is_fail 
        jp c, is_fail 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass

        ld de, msg_t47 
        call print 
        ld a, 0x80 
        dec a
        jp m, is_fail 
        jp po, is_fail 
        call is_pass

        ld de, msg_t48 
        call print 
        ld a, 0x01 
        dec a
        jp nz, is_fail 
        call is_pass

        ld de, msg_t49 
        call print 
        ld a, 0x10 
        dec a
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass

        ld de, msg_t50 
        call print 
        ld a, 0x02 
        dec a
        jp z, is_fail 
        call is_pass

        ; --- bloco 11: sla/sra/srl (51-55) ---
        ld de, msg_t51 
        call print 
        ld a, 0x80 
        sla a
        jp nz, is_fail 
        jp nc, is_fail 
        call is_pass

        ld de, msg_t52 
        call print 
        ld a, 0x81 
        sra a
        jp p, is_fail 
        jp nc, is_fail 
        cp 0xc0 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t53 
        call print 
        ld a, 0x01 
        srl a
        jp nz, is_fail 
        jp nc, is_fail 
        call is_pass

        ld de, msg_t54 
        call print 
        ld a, 0xff 
        sla a
        jp p, is_fail 
        jp nc, is_fail 
        cp 0xfe 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t55 
        call print 
        ld a, 0x08 
        sra a
        cp 0x04 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 12: rotacoes completas (56-60) ---
        ld de, msg_t56 
        call print 
        ld a, 0x80 
        rlc a
        jp nc, is_fail 
        cp 0x01 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t57 
        call print 
        ld a, 0x01 
        rrc a
        jp nc, is_fail 
        cp 0x80 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t58 
        call print 
        scf 
        ld a, 0x00 
        rl a
        jp c, is_fail 
        cp 0x01 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t59 
        call print 
        scf 
        ld a, 0x00 
        rr a
        jp c, is_fail 
        cp 0x80 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t60 
        call print 
        ld a, 0xff 
        rlc a
        jp nc, is_fail 
        cp 0xff 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 13: rotacoes de acumulador (61-65) ---
        ld de, msg_t61 
        call print 
        xor a 
        ld a, 0x80 
        rlca
        jp nc, is_fail 
        push af 
        pop hl 
        bit 6, l 
        jp z, is_fail 
        cp 0x01 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t62 
        call print 
        xor a 
        ld a, 0x01 
        rrca
        jp nc, is_fail 
        push af 
        pop hl 
        bit 6, l 
        jp z, is_fail 
        cp 0x80 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t63 
        call print 
        xor a 
        scf 
        ld a, 0x00 
        rla
        jp c, is_fail 
        cp 0x01 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t64 
        call print 
        xor a 
        scf 
        ld a, 0x00 
        rra
        jp c, is_fail 
        cp 0x80 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t65 
        call print 
        or a 
        scf
        jp nc, is_fail 
        call is_pass

        ; --- bloco 14: instrucoes diversas 8-bit (66-70) ---
        ld de, msg_t66 
        call print 
        scf 
        ccf
        jp c, is_fail 
        call is_pass

        ld de, msg_t67 
        call print 
        ld a, 0x00 
        cpl
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        cp 0xff 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t68 
        call print 
        ld a, 0x01 
        neg
        jp nc, is_fail 
        jp p, is_fail 
        push af 
        pop hl 
        bit 1, l 
        jp z, is_fail 
        cp 0xff 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t69 
        call print 
        ld a, 0x80 
        bit 7, a
        jp z, is_fail 
        call is_pass

        ld de, msg_t70 
        call print 
        ld a, 0xfe 
        bit 0, a
        jp nz, is_fail 
        call is_pass

        ; ============================================================
        ; NOVOS TESTES (16-BIT, BCD, MEMORIA) - 71 a 140
        ; ============================================================

        ; --- bloco 15: add hl, rr (71-75) (Preserva Z e S) ---
        ld de, msg_t71 
        call print
        ld a, 0x01 
        or a           ; A=1 -> Z=0
        ld hl, 0x0000 
        ld bc, 0x0000
        add hl, bc                  ; Result=0, mas ADD HL preserva Z
        jp z, is_fail               ; Z deve continuar 0
        jp c, is_fail
        ld a, h 
        cp 0x00 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t72 
        call print
        xor a                       ; A=0 -> Z=1
        ld hl, 0x8000 
        ld de, 0x8000
        add hl, de
        jp nz, is_fail              ; Z deve continuar 1, mesmo mudando HL
        jp nc, is_fail              ; 8000+8000 gera carry
        ld a, h 
        cp 0x00 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t73 
        call print
        ld hl, 0x4000 
        add hl, hl
        jp c, is_fail
        ld a, h 
        cp 0x80 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t74 
        call print
        ld hl, 0xffff 
        ld bc, 0x0001 
        add hl, bc
        jp nc, is_fail
        push af 
        pop de 
        bit 4, e 
        jp z, is_fail ; H=1 em carry de bit 11
        call is_pass

        ld de, msg_t75 
        call print
        ld hl, 0x1234 
        ld de, 0x1111 
        add hl, de
        ld a, h 
        cp 0x23 
        jp nz, is_fail
        ld a, l 
        cp 0x45 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 16: adc hl, rr (76-80) (Afeta TODAS as flags) ---
        ld de, msg_t76 
        call print
        scf 
        ld hl, 0x0000 
        ld bc, 0x0000 
        adc hl, bc
        jp z, is_fail               ; diferentemente de ADD HL, ADC afeta Z
        ld a, l 
        cp 0x01 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t77 
        call print
        scf 
        ld hl, 0xffff 
        ld de, 0x0000 
        adc hl, de
        jp nz, is_fail              ; result 0, logo Z=1
        jp nc, is_fail
        ld a, h 
        cp 0x00 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t78 
        call print
        scf 
        ld hl, 0x7fff 
        ld bc, 0x0000 
        adc hl, bc
        jp p, is_fail               ; S=1
        jp po, is_fail              ; overflow V=1
        ld a, h 
        cp 0x80 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t79 
        call print
        or a 
        ld hl, 0x1000 
        ld de, 0x2000 
        adc hl, de
        jp c, is_fail
        ld a, h 
        cp 0x30 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t80 
        call print
        scf 
        ld hl, 0x0fff 
        ld bc, 0x0000 
        adc hl, bc
        push af 
        pop de 
        bit 4, e 
        jp z, is_fail ; H=1
        ld a, h 
        cp 0x10 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 17: sbc hl, rr (81-85) ---
        ld de, msg_t81 
        call print
        or a 
        ld hl, 0x0000 
        ld bc, 0x0000 
        sbc hl, bc
        jp nz, is_fail 
        call is_pass

        ld de, msg_t82 
        call print
        scf 
        ld hl, 0x0000 
        ld de, 0x0000 
        sbc hl, de
        jp nc, is_fail
        jp p, is_fail               ; S=1
        ld a, h 
        cp 0xff 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t83 
        call print
        scf 
        ld hl, 0x8000 
        ld bc, 0x0000 
        sbc hl, bc
        jp m, is_fail               ; result 7FFF -> S=0
        jp po, is_fail              ; V=1 (overflow)
        call is_pass

        ld de, msg_t84 
        call print
        scf 
        ld hl, 0x1000 
        ld de, 0x0000 
        sbc hl, de
        push af 
        pop bc 
        bit 4, c 
        jp z, is_fail ; borrow (H=1)
        ld a, h 
        cp 0x0f 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t85 
        call print
        or a 
        ld hl, 0x1234 
        ld bc, 0x1234 
        sbc hl, bc
        jp nz, is_fail 
        call is_pass

        ; --- bloco 18: inc/dec 16-bit (86-90) (Nao alteram flags!) ---
        ld de, msg_t86 
        call print
        scf 
        ld bc, 0x0000 
        inc bc
        jp nc, is_fail              ; deve preservar C
        ld a, c 
        cp 0x01 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t87 
        call print
        or a 
        ld de, 0x0000 
        dec de
        jp c, is_fail               ; deve preservar C=0
        ld a, d 
        cp 0xff 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t88 
        call print
        xor a                       ; Z=1
        ld hl, 0xffff 
        inc hl
        jp nz, is_fail              ; Z deve continuar 1
        ld a, h 
        cp 0x00 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t89 
        call print
        ld a, 0x01 
        or a           ; Z=0
        ld bc, 0x0001 
        dec bc
        jp z, is_fail               ; Z deve continuar 0 mesmo BC sendo 0
        ld a, c 
        cp 0x00 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t90 
        call print
        scf 
        ld hl, 0x1234 
        inc hl 
        dec hl
        jp nc, is_fail
        ld a, l 
        cp 0x34 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 19: DAA - adicoes BCD (91-95) ---
        ld de, msg_t91 
        call print
        ld a, 0x15 
        add a, 0x22 
        daa
        jp c, is_fail 
        cp 0x37 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t92 
        call print
        ld a, 0x15 
        add a, 0x08 
        daa
        jp c, is_fail 
        cp 0x23 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t93 
        call print
        ld a, 0x80 
        add a, 0x80 
        daa
        jp nc, is_fail 
        cp 0x60 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t94 
        call print
        ld a, 0x99 
        add a, 0x99 
        daa
        jp nc, is_fail 
        cp 0x98 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t95 
        call print
        ld a, 0x09 
        add a, 0x09 
        daa
        jp c, is_fail 
        cp 0x18 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 20: DAA - subtracoes BCD (96-100) ---
        ld de, msg_t96 
        call print
        ld a, 0x25 
        sub 0x12 
        daa
        jp c, is_fail 
        cp 0x13 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t97 
        call print
        ld a, 0x22 
        sub 0x05 
        daa
        jp c, is_fail 
        cp 0x17 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t98 
        call print
        ld a, 0x12 
        sub 0x25 
        daa
        jp nc, is_fail 
        cp 0x87 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t99 
        call print
        ld a, 0x00 
        sub 0x01 
        daa
        jp nc, is_fail 
        cp 0x99 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t100 
        call print
        ld a, 0x10 
        sub 0x01 
        daa
        jp c, is_fail 
        cp 0x09 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 21: rld / rrd (101-105) ---
        ld de, msg_t101 
        call print
        ld hl, tmp_mem 
        ld [hl], 0x34 
        ld a, 0x12 
        rld
        jp pe, is_fail              ; a=0x13 (3 bits 1) -> PO
        jp m, is_fail               ; S=0
        jp z, is_fail               ; Z=0
        cp 0x13 
        jp nz, is_fail
        ld a, [hl] 
        cp 0x42 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t102 
        call print
        ld hl, tmp_mem 
        ld [hl], 0x00 
        ld a, 0x00 
        rld
        jp nz, is_fail              ; a=0 -> Z=1
        cp 0x00 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t103 
        call print
        ld hl, tmp_mem 
        ld [hl], 0x80 
        ld a, 0x00 
        rld
        jp m, is_fail               ; S deve ser 0! (Corrigido da falha)
        call is_pass

        ld de, msg_t104 
        call print
        ld hl, tmp_mem 
        ld [hl], 0x34 
        ld a, 0x12 
        rrd
        jp po, is_fail              ; a=0x14 (2 bits 1) -> PE
        cp 0x14 
        jp nz, is_fail
        ld a, [hl] 
        cp 0x23 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t105 
        call print
        ld hl, tmp_mem 
        ld [hl], 0x01 
        ld a, 0xff 
        rrd
        cp 0xf1 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 22: cpl, neg e ext (106-110) ---
        ld de, msg_t106 
        call print
        ld a, 0x80 
        neg
        jp p, is_fail               ; a=0x80 -> S=1
        jp po, is_fail              ; V=1 (overflow)
        jp nc, is_fail              ; C=1 (exceto se a=0)
        cp 0x80 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t107 
        call print
        ld a, 0x00 
        neg
        jp c, is_fail               ; A=0 -> C=0
        jp nz, is_fail              ; Z=1
        cp 0x00 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t108 
        call print
        ld a, 0xaa 
        cpl
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail ; cpl forca H=1
        bit 1, l 
        jp z, is_fail    ; cpl forca N=1
        ld a, a 
        cp 0x55 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t109 
        call print
        scf 
        ccf
        jp c, is_fail 
        call is_pass

        ld de, msg_t110 
        call print
        or a 
        ccf
        jp nc, is_fail 
        call is_pass

        ; --- bloco 23: IX / IY alu (111-115) ---
        ld de, msg_t111 
        call print
        ld ix, 0x1000 
        ld bc, 0x0234 
        add ix, bc
        push af 
        pop de 
        bit 1, e 
        jp nz, is_fail ; N=0 apos add ix
        push ix 
        pop hl
        ld a, h 
        cp 0x12 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t112 
        call print
        ld iy, 0xffff 
        ld de, 0x0001 
        add iy, de
        jp nc, is_fail
        push iy 
        pop hl
        ld a, h 
        cp 0x00 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t113 
        call print
        ld ix, 0x0000 
        inc ix 
        dec ix
        push ix 
        pop hl
        ld a, l 
        cp 0x00 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t114 
        call print
        ld iy, 0x8000 
        ld bc, 0x8000 
        add iy, bc
        jp nc, is_fail 
        call is_pass

        ld de, msg_t115 
        call print
        ld ix, 0x4000 
        add ix, ix
        push ix 
        pop hl
        ld a, h 
        cp 0x80 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 24: set, res (nao alteram flags) (116-120) ---
        ld de, msg_t116 
        call print
        scf 
        ld a, 0x00 
        set 7, a
        jp nc, is_fail
        cp 0x80 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t117 
        call print
        or a 
        ld a, 0xff 
        res 7, a
        jp c, is_fail
        cp 0x7f 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t118 
        call print
        ld a, 0x00 
        set 0, a
        cp 0x01 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t119 
        call print
        ld b, 0xff 
        res 0, b
        ld a, b 
        cp 0xfe 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t120 
        call print
        ld c, 0x00 
        set 4, c
        ld a, c 
        cp 0x10 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 25: inc/dec em memoria (121-125) ---
        ld de, msg_t121 
        call print
        ld hl, tmp_mem 
        ld [hl], 0xff 
        inc [hl]
        jp nz, is_fail
        ld a, [hl] 
        cp 0x00 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t122 
        call print
        ld hl, tmp_mem 
        ld [hl], 0x00 
        dec [hl]
        jp p, is_fail               ; S=1
        ld a, [hl] 
        cp 0xff 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t123 
        call print
        ld hl, tmp_mem 
        ld [hl], 0x7f 
        inc [hl]
        jp po, is_fail              ; V=1
        ld a, [hl] 
        cp 0x80 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t124 
        call print
        ld hl, tmp_mem 
        ld [hl], 0x80 
        dec [hl]
        jp po, is_fail              ; V=1
        ld a, [hl] 
        cp 0x7f 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t125 
        call print
        scf 
        ld hl, tmp_mem 
        ld [hl], 0x00 
        inc [hl]
        jp nc, is_fail              ; C deve continuar intacto
        call is_pass

        ; --- bloco 26: shifts na memoria (126-130) ---
        ld de, msg_t126 
        call print
        ld hl, tmp_mem 
        ld [hl], 0x80 
        sla [hl]
        jp nc, is_fail 
        jp nz, is_fail
        ld a, [hl] 
        cp 0x00 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t127 
        call print
        ld hl, tmp_mem 
        ld [hl], 0x81 
        sra [hl]
        jp nc, is_fail 
        jp p, is_fail
        ld a, [hl] 
        cp 0xc0 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t128 
        call print
        ld hl, tmp_mem 
        ld [hl], 0x01 
        srl [hl]
        jp nc, is_fail 
        jp nz, is_fail
        ld a, [hl] 
        cp 0x00 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t129 
        call print
        scf 
        ld hl, tmp_mem 
        ld [hl], 0x00 
        rl [hl]
        jp c, is_fail
        ld a, [hl] 
        cp 0x01 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t130 
        call print
        scf 
        ld hl, tmp_mem 
        ld [hl], 0x00 
        rr [hl]
        jp c, is_fail
        ld a, [hl] 
        cp 0x80 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 27/28: mix avancado (131-140) ---
        ld de, msg_t131 
        call print
        ld a, 0xff 
        add a, 0xff
        jp nc, is_fail 
        cp 0xfe 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t132 
        call print
        scf 
        ld a, 0x00 
        adc a, 0xff
        jp nc, is_fail 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t133 
        call print
        or a 
        ld a, 0x00 
        sbc a, 0xff
        jp nc, is_fail 
        cp 0x01 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t134 
        call print
        ld a, 0x80 
        sub 0xff
        jp pe, is_fail              ; V=1 (overflow)
        cp 0x81 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t135 
        call print
        ld a, 0x55 
        and 0xaa
        jp nz, is_fail 
        jp po, is_fail 
        call is_pass

        ld de, msg_t136 
        call print
        ld a, 0x55 
        or 0xaa
        jp p, is_fail 
        cp 0xff 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t137 
        call print
        ld a, 0xff 
        xor 0x0f
        jp p, is_fail 
        cp 0xf0 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t138 
        call print
        ld hl, 0x0001 
        add hl, hl 
        add hl, hl
        ld a, l 
        cp 0x04 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t139 
        call print
        scf 
        ld hl, 0xffff 
        adc hl, hl
        jp nc, is_fail
        ld a, h 
        cp 0xff 
        jp nz, is_fail 
        call is_pass

        ld de, msg_t140 
        call print
        scf 
        ld hl, 0x0000 
        sbc hl, hl
        jp nc, is_fail
        ld a, h 
        cp 0xff 
        jp nz, is_fail 
        call is_pass
        ; --- bloco 29: BIT 0-7, A (141-148) ---
        
        ld de, msg_t141 
        call print 
        ld a, 0x01 
        bit 0, a 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t142 
        call print 
        ld a, 0x02 
        bit 1, a 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t143 
        call print 
        ld a, 0x04 
        bit 2, a 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t144 
        call print 
        ld a, 0x08 
        bit 3, a 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t145 
        call print 
        ld a, 0x10 
        bit 4, a 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t146 
        call print 
        ld a, 0x20 
        bit 5, a 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t147 
        call print 
        ld a, 0x40 
        bit 6, a 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t148 
        call print 
        ld a, 0x80 
        bit 7, a 
        jp z, is_fail 
        call is_pass

        ; --- bloco 30: BIT 0-7, B (149-156) ---
        
        ld de, msg_t149 
        call print 
        ld b, 0x01 
        bit 0, b 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t150 
        call print 
        ld b, 0x02 
        bit 1, b 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t151 
        call print 
        ld b, 0x04 
        bit 2, b 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t152 
        call print 
        ld b, 0x08 
        bit 3, b 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t153 
        call print 
        ld b, 0x10 
        bit 4, b 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t154 
        call print 
        ld b, 0x20 
        bit 5, b 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t155 
        call print 
        ld b, 0x40 
        bit 6, b 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t156 
        call print 
        ld b, 0x80 
        bit 7, b 
        jp z, is_fail 
        call is_pass

        ; --- bloco 31: BIT 0-7, C (157-164) ---
        
        ld de, msg_t157 
        call print 
        ld c, 0xfe 
        bit 0, c 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t158 
        call print 
        ld c, 0xfd 
        bit 1, c 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t159 
        call print 
        ld c, 0xfb 
        bit 2, c 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t160 
        call print 
        ld c, 0xf7 
        bit 3, c 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t161 
        call print 
        ld c, 0xef 
        bit 4, c 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t162 
        call print 
        ld c, 0xdf 
        bit 5, c 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t163 
        call print 
        ld c, 0xbf 
        bit 6, c 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t164 
        call print 
        ld c, 0x7f 
        bit 7, c 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 32: BIT 0-7, D (165-172) ---
        
        ld de, msg_t165 
        call print 
        ld d, 0x00 
        bit 0, d 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t166 
        call print 
        ld d, 0x00 
        bit 1, d 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t167 
        call print 
        ld d, 0x00 
        bit 2, d 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t168 
        call print 
        ld d, 0x00 
        bit 3, d 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t169 
        call print 
        ld d, 0x00 
        bit 4, d 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t170 
        call print 
        ld d, 0x00 
        bit 5, d 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t171 
        call print 
        ld d, 0x00 
        bit 6, d 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t172 
        call print 
        ld d, 0x00 
        bit 7, d 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 33: SET 0-7, A (173-180) ---
        
        ld de, msg_t173 
        call print 
        ld a, 0x00 
        set 0, a 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t174 
        call print 
        ld a, 0x00 
        set 1, a 
        cp 0x02 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t175 
        call print 
        ld a, 0x00 
        set 2, a 
        cp 0x04 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t176 
        call print 
        ld a, 0x00 
        set 3, a 
        cp 0x08 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t177 
        call print 
        ld a, 0x00 
        set 4, a 
        cp 0x10 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t178 
        call print 
        ld a, 0x00 
        set 5, a 
        cp 0x20 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t179 
        call print 
        ld a, 0x00 
        set 6, a 
        cp 0x40 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t180 
        call print 
        ld a, 0x00 
        set 7, a 
        cp 0x80 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 34: SET 0-7, B (181-188) ---
        
        ld de, msg_t181 
        call print 
        ld b, 0x00 
        set 0, b 
        ld a, b 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t182 
        call print 
        ld b, 0x00 
        set 1, b 
        ld a, b 
        cp 0x02 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t183 
        call print 
        ld b, 0x00 
        set 2, b 
        ld a, b 
        cp 0x04 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t184 
        call print 
        ld b, 0x00 
        set 3, b 
        ld a, b 
        cp 0x08 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t185 
        call print 
        ld b, 0x00 
        set 4, b 
        ld a, b 
        cp 0x10 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t186 
        call print 
        ld b, 0x00 
        set 5, b 
        ld a, b 
        cp 0x20 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t187 
        call print 
        ld b, 0x00 
        set 6, b 
        ld a, b 
        cp 0x40 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t188 
        call print 
        ld b, 0x00 
        set 7, b 
        ld a, b 
        cp 0x80 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 35: SET 0-7, C (189-196) ---
        
        ld de, msg_t189 
        call print 
        ld c, 0xff 
        set 0, c 
        ld a, c 
        cp 0xff 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t190 
        call print 
        ld c, 0xff 
        set 1, c 
        ld a, c 
        cp 0xff 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t191 
        call print 
        ld c, 0xff 
        set 2, c 
        ld a, c 
        cp 0xff 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t192 
        call print 
        ld c, 0xff 
        set 3, c 
        ld a, c 
        cp 0xff 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t193 
        call print 
        ld c, 0xff 
        set 4, c 
        ld a, c 
        cp 0xff 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t194 
        call print 
        ld c, 0xff 
        set 5, c 
        ld a, c 
        cp 0xff 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t195 
        call print 
        ld c, 0xff 
        set 6, c 
        ld a, c 
        cp 0xff 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t196 
        call print 
        ld c, 0xff 
        set 7, c 
        ld a, c 
        cp 0xff 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 36: RES 0-7, A (197-204) ---
        
        ld de, msg_t197 
        call print 
        ld a, 0xff 
        res 0, a 
        cp 0xfe 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t198 
        call print 
        ld a, 0xff 
        res 1, a 
        cp 0xfd 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t199 
        call print 
        ld a, 0xff 
        res 2, a 
        cp 0xfb 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t200 
        call print 
        ld a, 0xff 
        res 3, a 
        cp 0xf7 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t201 
        call print 
        ld a, 0xff 
        res 4, a 
        cp 0xef 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t202 
        call print 
        ld a, 0xff 
        res 5, a 
        cp 0xdf 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t203 
        call print 
        ld a, 0xff 
        res 6, a 
        cp 0xbf 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t204 
        call print 
        ld a, 0xff 
        res 7, a 
        cp 0x7f 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 37: RES 0-7, B (205-212) ---
        
        ld de, msg_t205 
        call print 
        ld b, 0xff 
        res 0, b 
        ld a, b 
        cp 0xfe 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t206 
        call print 
        ld b, 0xff 
        res 1, b 
        ld a, b 
        cp 0xfd 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t207 
        call print 
        ld b, 0xff 
        res 2, b 
        ld a, b 
        cp 0xfb 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t208 
        call print 
        ld b, 0xff 
        res 3, b 
        ld a, b 
        cp 0xf7 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t209 
        call print 
        ld b, 0xff 
        res 4, b 
        ld a, b 
        cp 0xef 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t210 
        call print 
        ld b, 0xff 
        res 5, b 
        ld a, b 
        cp 0xdf 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t211 
        call print 
        ld b, 0xff 
        res 6, b 
        ld a, b 
        cp 0xbf 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t212 
        call print 
        ld b, 0xff 
        res 7, b 
        ld a, b 
        cp 0x7f 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 38: RES 0-7, C (213-220) ---
        
        ld de, msg_t213 
        call print 
        ld c, 0x00 
        res 0, c 
        ld a, c 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t214 
        call print 
        ld c, 0x00 
        res 1, c 
        ld a, c 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t215 
        call print 
        ld c, 0x00 
        res 2, c 
        ld a, c 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t216 
        call print 
        ld c, 0x00 
        res 3, c 
        ld a, c 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t217 
        call print 
        ld c, 0x00 
        res 4, c 
        ld a, c 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t218 
        call print 
        ld c, 0x00 
        res 5, c 
        ld a, c 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t219 
        call print 
        ld c, 0x00 
        res 6, c 
        ld a, c 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t220 
        call print 
        ld c, 0x00 
        res 7, c 
        ld a, c 
        cp 0x00 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 39: Flags via PUSH/POP AF (221-240) ---
        
        ld de, msg_t221 
        call print 
        ld hl, 0x0000 
        push hl 
        pop af 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t222 
        call print 
        ld hl, 0x0000 
        push hl 
        pop af 
        jp c, is_fail 
        call is_pass
        
        ld de, msg_t223 
        call print 
        ld hl, 0xffff 
        push hl 
        pop af 
        jp p, is_fail 
        call is_pass
        
        ld de, msg_t224 
        call print 
        ld hl, 0xffff 
        push hl 
        pop af 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t225 
        call print 
        ld hl, 0xffff 
        push hl 
        pop af 
        jp po, is_fail 
        call is_pass
        
        ld de, msg_t226 
        call print 
        ld hl, 0xffff 
        push hl 
        pop af 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t227 
        call print 
        ld hl, 0x0040 
        push hl 
        pop af 
        jp nz, is_fail 
        call is_pass ; Z flag (bit 6) = 1
        
        ld de, msg_t228 
        call print 
        ld hl, 0x0080 
        push hl 
        pop af 
        jp p, is_fail 
        call is_pass  ; S flag (bit 7) = 1
        
        ld de, msg_t229 
        call print 
        ld hl, 0x0001 
        push hl 
        pop af 
        jp nc, is_fail 
        call is_pass ; C flag (bit 0) = 1
        
        ld de, msg_t230 
        call print 
        ld hl, 0x0004 
        push hl 
        pop af 
        jp po, is_fail 
        call is_pass ; P/V flag (bit 2) = 1
        
        ld de, msg_t231 
        call print 
        ld hl, 0x0010 
        push hl 
        pop af 
        push af 
        pop de 
        bit 4, e 
        jp z, is_fail 
        call is_pass ; H flag
        
        ld de, msg_t232 
        call print 
        ld hl, 0x0002 
        push hl 
        pop af 
        push af 
        pop de 
        bit 1, e 
        jp z, is_fail 
        call is_pass ; N flag
        
        ld de, msg_t233 
        call print 
        ld a, 0x55 
        push af 
        pop hl 
        ld a, h 
        cp 0x55 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t234 
        call print 
        ld a, 0xaa 
        push af 
        pop hl 
        ld a, h 
        cp 0xaa 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t235 
        call print 
        or a 
        push af 
        pop hl 
        bit 0, l 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t236 
        call print 
        scf 
        push af 
        pop hl 
        bit 0, l 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t237 
        call print 
        ld a, 0xff 
        or a 
        push af 
        pop hl 
        bit 7, l 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t238 
        call print 
        xor a 
        push af 
        pop hl 
        bit 6, l 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t239 
        call print 
        ld a, 0x0f 
        add a, 0x01 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t240 
        call print 
        ld a, 0x01 
        sub 0x02 
        push af 
        pop hl 
        bit 1, l 
        jp z, is_fail 
        call is_pass

        ; --- bloco 40: Block Instructions Flags LDI/CPI (241-260) ---
        
        ld de, msg_t241 
        call print 
        ld bc, 0x0001 
        ld hl, tmp_mem 
        ld de, tmp_mem 
        ldi 
        jp pe, is_fail 
        call is_pass ; BC=0 -> P/V=0 (PO)
        
        ld de, msg_t242 
        call print 
        ld bc, 0x0002 
        ld hl, tmp_mem 
        ld de, tmp_mem 
        ldi 
        jp po, is_fail 
        call is_pass ; BC!=0 -> P/V=1 (PE)
        
        ld de, msg_t243 
        call print 
        ld bc, 0x0001 
        ld hl, tmp_mem 
        ld de, tmp_mem 
        ldi 
        push af 
        pop de 
        bit 4, e 
        jp nz, is_fail 
        call is_pass ; H=0
        
        ld de, msg_t244 
        call print 
        ld bc, 0x0001 
        ld hl, tmp_mem 
        ld de, tmp_mem 
        ldi 
        push af 
        pop de 
        bit 1, e 
        jp nz, is_fail 
        call is_pass ; N=0
        
        ld de, msg_t245 
        call print 
        ld bc, 0x0001 
        ld hl, tmp_mem 
        ld de, tmp_mem 
        ldd 
        jp pe, is_fail 
        call is_pass ; BC=0 -> PO
        
        ld de, msg_t246 
        call print 
        ld bc, 0x0002 
        ld hl, tmp_mem 
        ld de, tmp_mem 
        ldd 
        jp po, is_fail 
        call is_pass ; BC!=0 -> PE
        
        ld de, msg_t247 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x55 
        ld a, 0x55 
        ld bc, 0x0001 
        cpi 
        jp nz, is_fail 
        call is_pass ; Z=1 if A==[hl]
        
        ld de, msg_t248 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x55 
        ld a, 0x54 
        ld bc, 0x0001 
        cpi 
        jp z, is_fail 
        call is_pass ; Z=0 se !=
        
        ld de, msg_t249 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x00 
        ld a, 0x00 
        ld bc, 0x0001 
        cpi 
        jp pe, is_fail 
        call is_pass ; BC=0 -> PO
        
        ld de, msg_t250 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x00 
        ld a, 0x00 
        ld bc, 0x0002 
        cpi 
        jp po, is_fail 
        call is_pass ; BC!=0 -> PE
        
        ld de, msg_t251 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x00 
        ld a, 0x00 
        ld bc, 0x0001 
        cpd 
        push af 
        pop de 
        bit 1, e 
        jp z, is_fail 
        call is_pass ; N=1 in CPI/CPD
        
        ld de, msg_t252 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x55 
        ld a, 0x55 
        ld bc, 0x0001 
        cpd 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t253 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x55 
        ld a, 0x54 
        ld bc, 0x0001 
        cpd 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t254 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x00 
        ld a, 0x00 
        ld bc, 0x0001 
        cpd 
        jp pe, is_fail 
        call is_pass
        
        ld de, msg_t255 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x00 
        ld a, 0x00 
        ld bc, 0x0002 
        cpd 
        jp po, is_fail 
        call is_pass
        
        ld de, msg_t256 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x01 
        ld a, 0x00 
        ld bc, 0x0001 
        cpi 
        jp z, is_fail 
        call is_pass ; S=1 (0-1)
        
        ld de, msg_t257 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0xff 
        ld a, 0x00 
        ld bc, 0x0001 
        cpi 
        push af 
        pop de 
        bit 4, e 
        jp z, is_fail 
        call is_pass ; H=1
        
        ld de, msg_t258 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x01 
        ld a, 0x00 
        ld bc, 0x0001 
        cpd 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t259 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0xff 
        ld a, 0x00 
        ld bc, 0x0001 
        cpd 
        push af 
        pop de 
        bit 4, e 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t260 
        call print 
        ld bc, 0x0001 
        ld hl, tmp_mem 
        ld de, tmp_mem 
        ldi  
        jp z, is_fail 
        call is_pass ; Undocumented flag Y=0

        ; --- bloco 41: Indexadores IX/IY (261-280) ---
        
        ld de, msg_t261 
        call print 
        ld ix, 0x0000 
        ld bc, 0x1111 
        add ix, bc 
        push ix 
        pop hl 
        ld a, h 
        cp 0x11 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t262 
        call print 
        ld ix, 0x0000 
        ld de, 0x2222 
        add ix, de 
        push ix 
        pop hl 
        ld a, h 
        cp 0x22 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t263 
        call print 
        ld hl, 0
        add hl, sp
        ld ix, 0x1000 
        ld sp, 0x2000 
        add ix, sp
        ld sp, hl 
        push ix 
        pop hl 
        ld a, h 
        cp 0x30 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t264 
        call print 
        ld iy, 0x0000 
        ld bc, 0x4444 
        add iy, bc 
        push iy 
        pop hl 
        ld a, h 
        cp 0x44 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t265 
        call print 
        ld iy, 0x0000 
        ld de, 0x5555 
        add iy, de 
        push iy 
        pop hl 
        ld a, h 
        cp 0x55 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t266 
        call print 
        ld hl, 0
        add hl, sp
        ld iy, 0x1000 
        ld sp, 0x6000 
        add iy, sp 
        ld sp, hl
        push iy 
        pop hl 
        ld a, h 
        cp 0x70 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t267 
        call print 
        ld ix, 0xffff 
        ld bc, 0x0001 
        add ix, bc 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t268 
        call print 
        ld iy, 0xffff 
        ld de, 0x0001 
        add iy, de 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t269 
        call print 
        ld ix, 0x0fff 
        ld bc, 0x0001 
        add ix, bc 
        push af 
        pop de 
        bit 4, e 
        jp z, is_fail 
        call is_pass ; H=1 em add ix
        
        ld de, msg_t270 
        call print 
        ld iy, 0x0fff 
        ld bc, 0x0001 
        add iy, bc 
        push af 
        pop de 
        bit 4, e 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t271 
        call print 
        ld ix, 0x0000 
        inc ix 
        push ix 
        pop hl 
        ld a, l 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t272 
        call print 
        ld iy, 0x0000 
        dec iy 
        push iy 
        pop hl 
        ld a, h 
        cp 0xff 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t273 
        call print 
        ld hl, tmp_mem 
        ld ix, tmp_mem 
        push ix 
        pop de 
        ld a, h 
        cp d 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t274 
        call print 
        ld ix, tmp_mem 
        ld [ix+0], 0x99 
        ld a, [tmp_mem] 
        cp 0x99 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t275 
        call print 
        ld iy, tmp_mem 
        ld [iy+0], 0xaa 
        ld a, [tmp_mem] 
        cp 0xaa 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t276 
        call print 
        ld ix, tmp_mem 
        ld [ix+0], 0x00 
        inc [ix+0] 
        jp z, is_fail 
        call is_pass ; INC [ix+d] afeta Z

        ld de, msg_t277 
        call print 
        ld iy, tmp_mem 
        ld [iy+0], 0x00 
        dec [iy+0] 
        jp p, is_fail 
        call is_pass ; DEC [iy+d] afeta S
        
        ld de, msg_t278 
        call print 
        ld ix, tmp_mem 
        ld [ix+0], 0x7f 
        inc [ix+0] 
        jp po, is_fail 
        call is_pass ; overflow V=1
        
        ld de, msg_t279 
        call print 
        ld iy, tmp_mem 
        ld [iy+0], 0x80 
        dec [iy+0] 
        jp po, is_fail 
        call is_pass ; overflow V=1
        
        ld de, msg_t280 
        call print 
        scf 
        ld ix, tmp_mem 
        ld [ix+0], 0x00 
        inc [ix+0] 
        jp nc, is_fail 
        call is_pass ; Preserva C

        ; --- bloco 42: Shifts de Memoria HL / IX (281-300) ---
        
        ld de, msg_t281 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0xff 
        sla [hl] 
        jp p, is_fail 
        call is_pass
        
        ld de, msg_t282 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x01 
        sla [hl] 
        jp z, is_fail 
        jp c, is_fail 
        call is_pass
        
        ld de, msg_t283 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x80 
        sra [hl] 
        jp p, is_fail 
        jp c, is_fail 
        call is_pass
        
        ld de, msg_t284 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x81 
        sra [hl] 
        jp p, is_fail 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t285 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0xff 
        srl [hl] 
        jp m, is_fail 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t286 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x01 
        srl [hl] 
        jp nz, is_fail 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t287 
        call print 
        scf 
        ld hl, tmp_mem 
        ld [hl], 0x00 
        rl [hl] 
        jp c, is_fail 
        ld a, [hl] 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t288 
        call print 
        or a 
        ld hl, tmp_mem 
        ld [hl], 0x80 
        rl [hl] 
        jp nc, is_fail 
        ld a, [hl] 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t289 
        call print 
        scf 
        ld hl, tmp_mem 
        ld [hl], 0x00 
        rr [hl] 
        jp c, is_fail 
        ld a, [hl] 
        cp 0x80 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t290 
        call print 
        or a 
        ld hl, tmp_mem 
        ld [hl], 0x01 
        rr [hl] 
        jp nc, is_fail 
        ld a, [hl] 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t291 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x80 
        rlc [hl] 
        jp nc, is_fail 
        ld a, [hl] 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t292 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x01 
        rrc [hl] 
        jp nc, is_fail 
        ld a, [hl] 
        cp 0x80 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t293 
        call print 
        ld ix, tmp_mem 
        ld [ix+0], 0xff 
        sla [ix+0] 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t294 
        call print 
        ld iy, tmp_mem 
        ld [iy+0], 0xff 
        sra [iy+0] 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t295 
        call print 
        ld ix, tmp_mem 
        ld [ix+0], 0x01 
        srl [ix+0] 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t296 
        call print 
        scf 
        ld iy, tmp_mem 
        ld [iy+0], 0x00 
        rl [iy+0] 
        ld a, [iy+0] 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t297 
        call print 
        scf 
        ld ix, tmp_mem 
        ld [ix+0], 0x00 
        rr [ix+0] 
        ld a, [ix+0] 
        cp 0x80 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t298 
        call print 
        ld iy, tmp_mem 
        ld [iy+0], 0x80 
        rlc [iy+0] 
        ld a, [iy+0] 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t299 
        call print 
        ld ix, tmp_mem 
        ld [ix+0], 0x01 
        rrc [ix+0] 
        ld a, [ix+0] 
        cp 0x80 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t300 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x55 
        set 7, [hl] 
        ld a, [hl] 
        cp 0xd5 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 43: Borda e Logica Estendida (301-320) ---
        
        ld de, msg_t301 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0xaa 
        res 7, [hl] 
        ld a, [hl] 
        cp 0x2a 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t302 
        call print 
        ld ix, tmp_mem 
        ld [ix+0], 0x00
        set 3, [ix+0] 
        ld a, [ix+0] 
        push af
        call print_hex_byte
        pop af
        cp 0x08 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t303 
        call print 
        ld iy, tmp_mem 
        ld [iy+0], 0xff 
        res 3, [iy+0] 
        ld a, [iy+0] 
        cp 0xf7 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t304 
        call print 
        ld a, 0xff 
        add a, 0x01 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; H=1 em ADD
        
        ld de, msg_t305 
        call print 
        ld a, 0x00 
        sub 0x01 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; H=1 em SUB (borrow local)
        
        ld de, msg_t306 
        call print 
        scf 
        ld a, 0x0f 
        adc a, 0x00 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; H=1 em ADC
        
        ld de, msg_t307 
        call print 
        scf 
        ld a, 0x10 
        sbc a, 0x00 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; H=1 em SBC
        
        ld de, msg_t308 
        call print 
        ld a, 0x0f 
        inc a 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; H=1 em INC A
        
        ld de, msg_t309 
        call print 
        ld b, 0x10 
        dec b 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; H=1 em DEC B
        
        ld de, msg_t310 
        call print 
        ld c, 0x0f 
        inc c 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; H=1 em INC C
        
        ld de, msg_t311 
        call print 
        ld d, 0x10 
        dec d 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; H=1 em DEC D
        
        ld de, msg_t312 
        call print 
        ld e, 0x0f 
        inc e 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; H=1 em INC E
        
        ld de, msg_t313 
        call print 
        ld h, 0x10 
        dec h 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; H=1 em DEC H
        
        ld de, msg_t314 
        call print 
        ld l, 0x0f 
        inc l 
        push af 
        pop bc 
        bit 4, c 
        jp z, is_fail 
        call is_pass ; H=1 em INC L
        
        ld de, msg_t315 
        call print 
        ld a, 0x10 
        cp 0x11 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; H=1 em CP
        
        ld de, msg_t316 
        call print 
        xor a 
        neg 
        push af 
        pop hl 
        bit 4, l 
        jp nz, is_fail 
        call is_pass ; NEG 0 -> H=0
        
        ld de, msg_t317 
        call print 
        ld a, 0x01 
        neg 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; NEG 1 -> H=1
        
        ld de, msg_t318 
        call print 
        ld a, 0xff 
        and 0x00 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; AND -> H=1 obrigatorio
        
        ld de, msg_t319 
        call print 
        ld a, 0xff 
        or 0x00 
        push af 
        pop hl 
        bit 4, l 
        jp nz, is_fail 
        call is_pass ; OR -> H=0 obrigatorio
        
        ld de, msg_t320 
        call print 
        ld a, 0xff 
        xor 0x00 
        push af 
        pop hl 
        bit 4, l 
        jp nz, is_fail 
        call is_pass ; XOR -> H=0 obrigatorio

        ; --- bloco 44: Stack, Ext e DAA (321-340) ---
        
        ld de, msg_t321 
        call print 
        ld bc, 0x1234 
        push bc 
        pop hl 
        ld a, h 
        cp 0x12 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t322 
        call print 
        ld de, 0x5678 
        push de 
        pop hl 
        ld a, h 
        cp 0x56 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t323 
        call print 
        ld hl, 0x9abc 
        push hl 
        pop de 
        ld a, d 
        cp 0x9a 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t324 
        call print 
        ld ix, 0x1122 
        push ix 
        pop hl 
        ld a, h 
        cp 0x11 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t325 
        call print 
        ld iy, 0x3344 
        push iy 
        pop hl 
        ld a, h 
        cp 0x33 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t326 
        call print 
        ld hl, 0x0000 
        push hl 
        pop af 
        scf 
        push af 
        pop hl 
        bit 0, l 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t327 
        call print 
        ld hl, 0xffff 
        push hl 
        pop af 
        or a 
        push af 
        pop hl 
        bit 0, l 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t328 
        call print 
        ld a, 0x99 
        add a, 0x00 
        daa 
        jp c, is_fail 
        cp 0x99 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t329 
        call print 
        ld a, 0x00 
        sub 0x99 
        daa 
        jp nc, is_fail 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t330 
        call print 
        ld a, 0x01 
        sub 0x02 
        daa 
        jp nc, is_fail 
        cp 0x99 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t331 
        call print 
        ld a, 0x50 
        add a, 0x50 
        daa 
        jp nc, is_fail 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t332 
        call print 
        ld a, 0x10 
        sub 0x20 
        daa 
        jp nc, is_fail 
        cp 0x90 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t333 
        call print 
        scf 
        ccf 
        ccf 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t334 
        call print 
        or a 
        ccf 
        ccf 
        jp c, is_fail 
        call is_pass
        
        ld de, msg_t335 
        call print 
        ld bc, 0xffff 
        inc bc 
        inc bc 
        ld a, c 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t336 
        call print 
        ld de, 0x0000 
        dec de 
        dec de 
        ld a, e 
        cp 0xfe 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t337 
        call print 
        ld hl, 0xffff 
        inc hl 
        inc hl 
        ld a, l 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t338 
        call print 
        ld hl, 0
        add hl, sp
        ld sp, 0xffff 
        inc sp 
        ld sp, hl
        ld a, 0x00 
        push af 
        pop bc 
        ld a, b 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t339 
        call print 
        scf 
        ld a, 0x00 
        sbc a, 0x00 
        daa 
        jp nc, is_fail 
        cp 0x99 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t340 
        call print 
        or a 
        ld a, 0x00 
        adc a, 0x00 
        daa 
        jp c, is_fail 
        cp 0x00 
        jp nz, is_fail 
        call is_pass

        ; --- bloco 45: Random Misc Edge Cases (341-356) ---
        
        ld de, msg_t341 
        call print 
        ld b, 0x00 
        djnz $+4 
        jp is_fail 
        call is_pass ; b=0 -> dec b -> b=FF -> jump
        
        ld de, msg_t342 
        call print 
        ld b, 0x01 
        djnz $+5 
        call is_pass 
        jp $+6
        jp is_fail ; b=1 -> dec b -> b=0 -> no jump
        
        ld de, msg_t343 
        call print 
        ld c, 0xff 
        inc c 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t344 
        call print 
        ld c, 0x00 
        dec c 
        jp p, is_fail 
        call is_pass
        
        ld de, msg_t345 
        call print 
        ld a, 5
        ex af, af' 
        ld a, 6
        ex af, af' 
        cp a, 6
        jp z, is_fail
        call is_pass
        
        ld de, msg_t346 
        call print 
        ld c, 5
        exx 
        ld c, 6
        exx 
        ld a, c
        cp 6
        jp z, is_fail
        call is_pass
        
        ld de, msg_t347 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0xff 
        dec [hl] 
        jp p, is_fail 
        call is_pass
        
        ld de, msg_t348 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x00 
        inc [hl] 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t349 
        call print 
        ld ix, tmp_mem 
        ld [ix+0], 0xff 
        dec [ix+0] 
        jp p, is_fail 
        call is_pass
        
        ld de, msg_t350 
        call print 
        ld iy, tmp_mem 
        ld [iy+0], 0x00 
        inc [iy+0] 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t351 
        call print 
        ld a, 0xff 
        rla 
        rra 
        cp 0xff 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t352 
        call print 
        ld a, 0x00 
        rla 
        rra 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t353 
        call print 
        scf 
        ld a, 0xff 
        rla 
        rra 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t354 
        call print 
        or a 
        ld a, 0x00 
        rla 
        rra 
        jp c, is_fail 
        call is_pass
        
        ld de, msg_t355 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x00 
        ld a, 0xff 
        rld 
        rrd 
        cp 0xff 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t356 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0xff 
        ld a, 0x00 
        rrd 
        rld 
        cp 0x00 
        jp nz, is_fail 
        call is_pass

        ; --- Bloco 46: ADD A, reg (357-364) ---
        
        ld de, msg_t357 
        call print 
        ld a, 0x10 
        ld b, 0x05 
        add a, b 
        cp 0x15 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t358 
        call print 
        ld a, 0x10 
        ld c, 0x05 
        add a, c 
        cp 0x15 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t359 
        call print 
        ld a, 0x10 
        ld d, 0x05 
        add a, d 
        cp 0x15 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t360 
        call print 
        ld a, 0x10 
        ld e, 0x05 
        add a, e 
        cp 0x15 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t361 
        call print 
        ld a, 0x10 
        ld h, 0x05 
        add a, h 
        cp 0x15 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t362 
        call print 
        ld a, 0x10 
        ld l, 0x05 
        add a, l 
        cp 0x15 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t363 
        call print 
        ld a, 0x10 
        add a, a 
        cp 0x20 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t364 
        call print 
        ld a, 0x10 
        ld hl, tmp_mem 
        ld [hl], 0x05 
        add a, [hl] 
        cp 0x15 
        jp nz, is_fail 
        call is_pass

        ; --- Bloco 47: ADC A, reg (365-372) ---
        
        ld de, msg_t365 
        call print 
        scf 
        ld a, 0x10 
        ld b, 0x05 
        adc a, b 
        cp 0x16 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t366 
        call print 
        or a 
        ld a, 0x10 
        ld c, 0x05 
        adc a, c 
        cp 0x15 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t367 
        call print 
        scf 
        ld a, 0x10 
        ld d, 0x05 
        adc a, d 
        cp 0x16 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t368 
        call print 
        or a 
        ld a, 0x10 
        ld e, 0x05 
        adc a, e 
        cp 0x15 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t369 
        call print 
        scf 
        ld a, 0x10 
        ld h, 0x05 
        adc a, h 
        cp 0x16 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t370 
        call print 
        or a 
        ld a, 0x10 
        ld l, 0x05 
        adc a, l 
        cp 0x15 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t371 
        call print 
        scf 
        ld a, 0x10 
        adc a, a 
        cp 0x21 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t372 
        call print 
        scf 
        ld a, 0x10 
        ld hl, tmp_mem 
        ld [hl], 0x05 
        adc a, [hl] 
        cp 0x16 
        jp nz, is_fail 
        call is_pass

        ; --- Bloco 48: SUB reg (373-380) ---
        
        ld de, msg_t373 
        call print 
        ld a, 0x10 
        ld b, 0x05 
        sub b 
        cp 0x0B 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t374 
        call print 
        ld a, 0x10 
        ld c, 0x05 
        sub c 
        cp 0x0B 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t375 
        call print 
        ld a, 0x10 
        ld d, 0x05 
        sub d 
        cp 0x0B 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t376 
        call print 
        ld a, 0x10 
        ld e, 0x05 
        sub e 
        cp 0x0B 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t377 
        call print 
        ld a, 0x10 
        ld h, 0x05 
        sub h 
        cp 0x0B 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t378 
        call print 
        ld a, 0x10 
        ld l, 0x05 
        sub l 
        cp 0x0B 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t379 
        call print 
        ld a, 0x10 
        sub a 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t380 
        call print 
        ld a, 0x10 
        ld hl, tmp_mem 
        ld [hl], 0x05 
        sub [hl] 
        cp 0x0B 
        jp nz, is_fail 
        call is_pass

        ; --- Bloco 49: SBC A, reg (381-388) ---
        
        ld de, msg_t381 
        call print 
        scf 
        ld a, 0x10 
        ld b, 0x05 
        sbc a, b 
        cp 0x0A 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t382 
        call print 
        or a 
        ld a, 0x10 
        ld c, 0x05 
        sbc a, c 
        cp 0x0B 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t383 
        call print 
        scf 
        ld a, 0x10 
        ld d, 0x05 
        sbc a, d 
        cp 0x0A 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t384 
        call print 
        or a 
        ld a, 0x10 
        ld e, 0x05 
        sbc a, e 
        cp 0x0B 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t385 
        call print 
        scf 
        ld a, 0x10 
        ld h, 0x05 
        sbc a, h 
        cp 0x0A 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t386 
        call print 
        or a 
        ld a, 0x10 
        ld l, 0x05 
        sbc a, l 
        cp 0x0B 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t387 
        call print 
        scf 
        ld a, 0x10 
        sbc a, a 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t388 
        call print 
        scf 
        ld a, 0x10 
        ld hl, tmp_mem 
        ld [hl], 0x05 
        sbc a, [hl] 
        cp 0x0A 
        jp nz, is_fail 
        call is_pass

        ; --- Bloco 50: AND reg (389-396) ---
        
        ld de, msg_t389 
        call print 
        ld a, 0xF0 
        ld b, 0x33 
        and b 
        cp 0x30 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t390 
        call print 
        ld a, 0xF0 
        ld c, 0x33 
        and c 
        cp 0x30 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t391 
        call print 
        ld a, 0xF0 
        ld d, 0x33 
        and d 
        cp 0x30 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t392 
        call print 
        ld a, 0xF0 
        ld e, 0x33 
        and e 
        cp 0x30 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t393 
        call print 
        ld a, 0xF0 
        ld h, 0x33 
        and h 
        cp 0x30 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t394 
        call print 
        ld a, 0xF0 
        ld l, 0x33 
        and l 
        cp 0x30 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t395 
        call print 
        ld a, 0xF0 
        and a 
        cp 0xF0 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t396 
        call print 
        ld a, 0xF0 
        ld hl, tmp_mem 
        ld [hl], 0x33 
        and [hl] 
        cp 0x30 
        jp nz, is_fail 
        call is_pass

        ; --- Bloco 51: XOR reg (397-404) ---
        
        ld de, msg_t397 
        call print 
        ld a, 0xF0 
        ld b, 0x33 
        xor b 
        cp 0xC3 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t398 
        call print 
        ld a, 0xF0 
        ld c, 0x33 
        xor c 
        cp 0xC3 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t399 
        call print 
        ld a, 0xF0 
        ld d, 0x33 
        xor d 
        cp 0xC3 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t400 
        call print 
        ld a, 0xF0 
        ld e, 0x33 
        xor e 
        cp 0xC3 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t401 
        call print 
        ld a, 0xF0 
        ld h, 0x33 
        xor h 
        cp 0xC3 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t402 
        call print 
        ld a, 0xF0 
        ld l, 0x33 
        xor l 
        cp 0xC3 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t403 
        call print 
        ld a, 0xF0 
        xor a 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t404 
        call print 
        ld a, 0xF0 
        ld hl, tmp_mem 
        ld [hl], 0x33 
        xor [hl] 
        cp 0xC3 
        jp nz, is_fail 
        call is_pass

        ; --- Bloco 52: OR reg (405-412) ---
        
        ld de, msg_t405 
        call print 
        ld a, 0x10 
        ld b, 0x01 
        or b 
        cp 0x11 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t406 
        call print 
        ld a, 0x10 
        ld c, 0x01 
        or c 
        cp 0x11 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t407 
        call print 
        ld a, 0x10 
        ld d, 0x01 
        or d 
        cp 0x11 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t408 
        call print 
        ld a, 0x10 
        ld e, 0x01 
        or e 
        cp 0x11 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t409 
        call print 
        ld a, 0x10 
        ld h, 0x01 
        or h 
        cp 0x11 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t410 
        call print 
        ld a, 0x10 
        ld l, 0x01 
        or l 
        cp 0x11 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t411 
        call print 
        ld a, 0x10 
        or a 
        cp 0x10 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t412 
        call print 
        ld a, 0x10 
        ld hl, tmp_mem 
        ld [hl], 0x01 
        or [hl] 
        cp 0x11 
        jp nz, is_fail 
        call is_pass

        ; --- Bloco 53: CP reg (413-420) ---
        
        ld de, msg_t413 
        call print 
        ld a, 0x10 
        ld b, 0x10 
        cp b 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t414 
        call print 
        ld a, 0x10 
        ld c, 0x0F 
        cp c 
        jp c, is_fail 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t415 
        call print 
        ld a, 0x10 
        ld d, 0x11 
        cp d 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t416 
        call print 
        ld a, 0x10 
        ld e, 0x10 
        cp e 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t417 
        call print 
        ld a, 0x10 
        ld h, 0x0F 
        cp h 
        jp m, is_fail 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t418 
        call print 
        ld a, 0x10 
        ld l, 0x11 
        cp l 
        jp p, is_fail 
        call is_pass
        
        ld de, msg_t419 
        call print 
        ld a, 0x10 
        cp a 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t420 
        call print 
        ld a, 0x10 
        ld hl, tmp_mem 
        ld [hl], 0x10 
        cp [hl] 
        jp nz, is_fail 
        call is_pass

        ; --- Bloco 54: ALU Imediato Flag Edge Cases (421-484) ---
        ; ADD A, n (421-428)
        
        ld de, msg_t421 
        call print 
        ld a, 0x00 
        add a, 0x00 
        jp nz, is_fail 
        jp c, is_fail 
        call is_pass
        
        ld de, msg_t422 
        call print 
        ld a, 0xFF 
        add a, 0x01 
        jp nz, is_fail 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t423 
        call print 
        ld a, 0x0F 
        add a, 0x01 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; H=1
        
        ld de, msg_t424 
        call print 
        ld a, 0x7F 
        add a, 0x01 
        jp p, is_fail 
        jp po, is_fail 
        call is_pass ; S=1, V=1
        
        ld de, msg_t425 
        call print 
        ld a, 0x80 
        add a, 0xFF 
        jp m, is_fail 
        jp po, is_fail 
        call is_pass ; S=0, V=1, C=1
        
        ld de, msg_t426 
        call print 
        ld a, 0x55 
        add a, 0xAA 
        jp z, is_fail 
        jp p, is_fail 
        call is_pass ; 0xFF -> Z=0, S=1
        
        ld de, msg_t427 
        call print 
        ld a, 0x01 
        add a, 0xFF 
        jp nz, is_fail 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t428 
        call print 
        ld a, 0x7F 
        add a, 0x7F 
        jp p, is_fail 
        jp po, is_fail 
        call is_pass ; FE -> S=1, V=1

        ; ADC A, n (429-436)
        
        ld de, msg_t429 
        call print 
        scf 
        ld a, 0x00 
        adc a, 0x00 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t430 
        call print 
        or a 
        ld a, 0xFF 
        adc a, 0x00 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t431 
        call print 
        scf 
        ld a, 0xFF 
        adc a, 0x00 
        jp nc, is_fail 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t432 
        call print 
        scf 
        ld a, 0x0F 
        adc a, 0x00 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t433 
        call print 
        scf 
        ld a, 0x7F 
        adc a, 0x00 
        jp p, is_fail 
        jp po, is_fail 
        call is_pass
        
        ld de, msg_t434 
        call print 
        scf 
        ld a, 0x80 
        adc a, 0xFF 
        jp po, is_fail 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t435 
        call print 
        or a 
        ld a, 0x80 
        adc a, 0x80 
        jp nz, is_fail 
        jp nc, is_fail 
        jp po, is_fail 
        call is_pass
        
        ld de, msg_t436 
        call print 
        scf 
        ld a, 0x7F 
        adc a, 0x7F 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass

        ; SUB n (437-444)
        
        ld de, msg_t437 
        call print 
        ld a, 0x00 
        sub 0x00 
        jp nz, is_fail 
        jp c, is_fail 
        call is_pass
        
        ld de, msg_t438 
        call print 
        ld a, 0x00 
        sub 0x01 
        jp nc, is_fail 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t439 
        call print 
        ld a, 0x10 
        sub 0x01 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; H=1
        
        ld de, msg_t440 
        call print 
        ld a, 0x80 
        sub 0x01 
        jp m, is_fail 
        jp po, is_fail 
        call is_pass ; S=0, V=1
        
        ld de, msg_t441 
        call print 
        ld a, 0x7F 
        sub 0xFF 
        jp p, is_fail 
        jp po, is_fail 
        call is_pass ; S=1, V=1, C=1
        
        ld de, msg_t442 
        call print 
        ld a, 0xAA 
        sub 0x55 
        cp 0x55 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t443 
        call print 
        ld a, 0x55 
        sub 0xAA 
        jp nc, is_fail 
        cp 0xAB 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t444 
        call print 
        ld a, 0x00 
        sub 0x80 
        jp p, is_fail 
        jp po, is_fail 
        jp nc, is_fail 
        call is_pass ; S=1, V=1, C=1

        ; SBC A, n (445-452)
        
        ld de, msg_t445 
        call print 
        scf 
        ld a, 0x01 
        sbc a, 0x00 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t446 
        call print 
        or a 
        ld a, 0x00 
        sbc a, 0x01 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t447 
        call print 
        scf 
        ld a, 0x00 
        sbc a, 0x00 
        jp nc, is_fail 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t448 
        call print 
        scf 
        ld a, 0x10 
        sbc a, 0x00 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t449 
        call print 
        scf 
        ld a, 0x80 
        sbc a, 0x00 
        jp m, is_fail 
        jp po, is_fail 
        call is_pass
        
        ld de, msg_t450 
        call print 
        scf 
        ld a, 0x7F 
        sbc a, 0xFF 
        jp m, is_fail 
        jp po, is_fail 
        call is_pass
        
        ld de, msg_t451 
        call print 
        or a 
        ld a, 0x80 
        sbc a, 0x80 
        jp nz, is_fail 
        jp c, is_fail 
        call is_pass
        
        ld de, msg_t452 
        call print 
        scf 
        ld a, 0x00 
        sbc a, 0x7F 
        jp p, is_fail 
        jp po, is_fail
        cp 0x80
        jp nz, is_fail 
        call is_pass

        ; AND n (453-460)
        
        ld de, msg_t453 
        call print 
        ld a, 0xFF 
        and 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t454 
        call print 
        ld a, 0xFF 
        and 0xFF 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t455 
        call print 
        ld a, 0xAA 
        and 0x55 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t456 
        call print 
        ld a, 0x55 
        and 0xAA 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t457 
        call print 
        ld a, 0xFF 
        and 0x0F 
        cp 0x0F 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t458 
        call print 
        ld a, 0x0F 
        and 0xF0 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t459 
        call print 
        ld a, 0xFF 
        and 0x80 
        jp p, is_fail 
        call is_pass ; S=1
        
        ld de, msg_t460 
        call print 
        ld a, 0xFF 
        and 0x01 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; H=1 obrigatorio

        ; XOR n (461-468)
        
        ld de, msg_t461 
        call print 
        ld a, 0xFF 
        xor 0x00 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t462 
        call print 
        ld a, 0xFF 
        xor 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t463 
        call print 
        ld a, 0xAA 
        xor 0x55 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t464 
        call print 
        ld a, 0x55 
        xor 0xAA 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t465 
        call print 
        ld a, 0xFF 
        xor 0x0F 
        cp 0xF0 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t466 
        call print 
        ld a, 0x0F 
        xor 0xF0 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t467 
        call print 
        ld a, 0x7F 
        xor 0xFF 
        jp p, is_fail 
        call is_pass ; S=1
        
        ld de, msg_t468 
        call print 
        ld a, 0xFF 
        xor 0x00 
        push af 
        pop hl 
        bit 4, l 
        jp nz, is_fail 
        call is_pass ; H=0 obrigatorio

        ; OR n (469-476)
        
        ld de, msg_t469 
        call print 
        ld a, 0x00 
        or 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t470 
        call print 
        ld a, 0x00 
        or 0xFF 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t471 
        call print 
        ld a, 0xAA 
        or 0x55 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t472 
        call print 
        ld a, 0x55 
        or 0xAA 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t473 
        call print 
        ld a, 0xF0 
        or 0x0F 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t474 
        call print 
        ld a, 0x0F 
        or 0xF0 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t475 
        call print 
        ld a, 0x00 
        or 0x80 
        jp p, is_fail 
        call is_pass ; S=1
        
        ld de, msg_t476 
        call print 
        ld a, 0xFF 
        or 0x00 
        push af 
        pop hl 
        bit 4, l 
        jp nz, is_fail 
        call is_pass ; H=0 obrigatorio

        ; CP n (477-484)
        
        ld de, msg_t477 
        call print 
        ld a, 0x00 
        cp 0x00 
        jp nz, is_fail 
        jp c, is_fail 
        call is_pass
        
        ld de, msg_t478 
        call print 
        ld a, 0x00 
        cp 0x01 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t479 
        call print 
        ld a, 0x10 
        cp 0x01 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; H=1
        
        ld de, msg_t480 
        call print 
        ld a, 0x80 
        cp 0x01 
        jp m, is_fail 
        jp po, is_fail 
        call is_pass ; S=0, V=1
        
        ld de, msg_t481 
        call print 
        ld a, 0x7F 
        cp 0xFF 
        jp p, is_fail 
        jp po, is_fail 
        call is_pass ; S=1, V=1, C=1
        
        ld de, msg_t482 
        call print 
        ld a, 0xAA 
        cp 0x55 
        jp c, is_fail 
        call is_pass
        
        ld de, msg_t483 
        call print 
        ld a, 0x55 
        cp 0xAA 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t484 
        call print 
        ld a, 0x00 
        cp 0x80 
        jp p, is_fail 
        jp po, is_fail 
        jp nc, is_fail 
        call is_pass ; S=1, V=1, C=1

        ; --- Bloco 55: ADD HL, reg (485-488) ---
        
        ld de, msg_t485 
        call print 
        ld hl, 0x1000 
        ld bc, 0x2000 
        add hl, bc 
        push hl 
        pop de 
        ld a, d 
        cp 0x30 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t486 
        call print 
        ld hl, 0x1000 
        ld de, 0x2000 
        add hl, de 
        push hl 
        pop de 
        ld a, d 
        cp 0x30 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t487 
        call print 
        ld hl, 0x1000 
        add hl, hl 
        push hl 
        pop de 
        ld a, d 
        cp 0x20 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t488 
        call print 
        ld ix, 0
        add ix, sp
        ld hl, 0x1000 
        ld sp, 0x2000 
        add hl, sp 
        ld sp, ix
        push hl 
        pop de 
        ld a, d 
        cp 0x30 
        jp nz, is_fail 
        call is_pass

        ; --- Bloco 56: ADC HL, reg (489-492) ---
        
        ld de, msg_t489 
        call print 
        scf 
        ld hl, 0x1000 
        ld bc, 0x2000 
        adc hl, bc 
        ld a, l 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t490 
        call print 
        or a 
        ld hl, 0x1000 
        ld de, 0x2000 
        adc hl, de 
        push hl 
        pop de 
        ld a, d 
        cp 0x30 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t491 
        call print 
        scf 
        ld hl, 0x1000 
        adc hl, hl 
        ld a, l 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t492 
        call print 
        or a 
        ld hl, 0x1000 
        ld sp, 0x2000 
        adc hl, sp 
        push hl 
        pop de 
        ld a, d 
        cp 0x30 
        jp nz, is_fail 
        call is_pass

        ; --- Bloco 57: SBC HL, reg (493-496) ---
        
        ld de, msg_t493 
        call print 
        scf 
        ld hl, 0x2000 
        ld bc, 0x1000 
        sbc hl, bc 
        push hl 
        pop de 
        ld a, d 
        cp 0x0F 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t494 
        call print 
        or a 
        ld hl, 0x2000 
        ld de, 0x1000 
        sbc hl, de 
        push hl 
        pop de 
        ld a, d 
        cp 0x10 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t495 
        call print 
        scf 
        ld hl, 0x2000 
        sbc hl, hl 
        push hl 
        pop de 
        ld a, d 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t496 
        call print 
        or a 
        ld hl, 0x2000 
        ld sp, 0x1000 
        sbc hl, sp 
        push hl 
        pop de 
        ld a, d 
        cp 0x10 
        jp nz, is_fail 
        call is_pass

        ; --- Bloco 58: 16-bit Flags (497-508) ---
        
        ld de, msg_t497 
        call print 
        scf
        ld hl, 0x0FFF 
        ld bc, 0x0001 
        add hl, bc 
        push af 
        pop de 
        bit 4, e 
        jp z, is_fail 
        call is_pass ; ADD HL H=1
        
        ld de, msg_t498 
        call print 
        scf
        ld hl, 0xFFFF 
        ld bc, 0x0001 
        add hl, bc 
        jp nc, is_fail 
        call is_pass ; ADD HL C=1
        
        ld de, msg_t499 
        call print 
        scf
        ld hl, 0x0FFF 
        ld bc, 0x0001 
        adc hl, bc 
        push af 
        pop de 
        bit 4, e 
        jp z, is_fail 
        call is_pass ; ADC HL H=1
        
        ld de, msg_t500 
        call print 
        or a
        ld hl, 0xFFFF 
        ld bc, 0x0000 
        adc hl, bc 
        jp c, is_fail 
        call is_pass ; ADC HL C=0
        
        ld de, msg_t501 
        call print 
        scf
        ld hl, 0xFFFF 
        ld bc, 0x0001 
        adc hl, bc 
        jp nc, is_fail 
        call is_pass ; ADC HL C=1
        
        ld de, msg_t502 
        call print 
        scf
        ld hl, 0x8000 
        ld bc, 0x8000 
        adc hl, bc 
        jp po, is_fail 
        call is_pass ; ADC HL V=1 (Overflow)
        
        ld de, msg_t503 
        call print 
        scf
        ld hl, 0x1000 
        ld bc, 0x0001 
        sbc hl, bc 
        push af 
        pop de 
        bit 4, e 
        jp z, is_fail 
        call is_pass ; SBC HL H=1
        
        ld de, msg_t504 
        call print 
        scf
        ld hl, 0x0000 
        ld bc, 0x0001 
        sbc hl, bc 
        jp nc, is_fail 
        call is_pass ; SBC HL C=1
        
        ld de, msg_t505 
        call print 
        scf
        ld hl, 0x7FFF 
        ld bc, 0xFFFF 
        sbc hl, bc 
        jp pe, is_fail 
        call is_pass ; SBC HL V=1 (Overflow)
        
        ld de, msg_t506 
        call print 
        or a
        ld hl, 0x1000 
        ld bc, 0x1000 
        sbc hl, bc 
        jp nz, is_fail 
        call is_pass ; SBC HL Z=1
        
        ld de, msg_t507 
        call print 
        ld hl, 0x1000 
        ld bc, 0x1000 
        adc hl, bc 
        jp z, is_fail 
        call is_pass ; ADC HL afeta Z (Z=0)
        
        ld de, msg_t508 
        call print 
        ld hl, 0x0000 
        ld bc, 0x0000 
        adc hl, bc 
        jp nz, is_fail 
        call is_pass ; ADC HL Z=1

        ; --- Bloco 59: IX/IY ALU Base (509-524) ---
        
        ld de, msg_t509 
        call print 
        ld ix, 0x1000 
        ld bc, 0x2000 
        add ix, bc 
        push ix 
        pop de 
        ld a, d 
        cp 0x30 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t510 
        call print 
        ld ix, 0x1000 
        ld de, 0x2000 
        add ix, de 
        push ix 
        pop de 
        ld a, d 
        cp 0x30 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t511 
        call print 
        ld ix, 0x1000 
        add ix, ix 
        push ix 
        pop de 
        ld a, d 
        cp 0x20 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t512 
        call print 
        ld ix, 0x1000 
        ld sp, 0x2000 
        add ix, sp 
        push ix 
        pop de 
        ld a, d 
        cp 0x30 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t513 
        call print 
        ld iy, 0x1000 
        ld bc, 0x2000 
        add iy, bc 
        push iy 
        pop de 
        ld a, d 
        cp 0x30 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t514 
        call print 
        ld iy, 0x1000 
        ld de, 0x2000 
        add iy, de 
        push iy 
        pop de 
        ld a, d 
        cp 0x30 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t515 
        call print 
        ld iy, 0x1000 
        add iy, iy 
        push iy 
        pop de 
        ld a, d 
        cp 0x20 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t516 
        call print 
        ld iy, 0x1000 
        ld sp, 0x2000 
        add iy, sp 
        push iy 
        pop de 
        ld a, d 
        cp 0x30 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t517 
        call print 
        ld ix, 0xFFFF 
        ld bc, 0x0001 
        add ix, bc 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t518 
        call print 
        ld iy, 0xFFFF 
        ld de, 0x0001 
        add iy, de 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t519 
        call print 
        ld ix, 0x0FFF 
        ld bc, 0x0001 
        add ix, bc 
        push af 
        pop de 
        bit 4, e 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t520 
        call print 
        ld iy, 0x0FFF 
        ld de, 0x0001 
        add iy, de 
        push af 
        pop de 
        bit 4, e 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t521 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x05 
        ld a, 0x10 
        add a, [hl] 
        cp 0x15 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t522 
        call print 
        ld ix, tmp_mem 
        ld [ix+0], 0x05 
        ld a, 0x10 
        adc a, [ix+0] 
        cp 0x15 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t523 
        call print 
        ld iy, tmp_mem 
        ld [iy+0], 0x05 
        ld a, 0x10 
        sub [iy+0] 
        cp 0x0B 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t524 
        call print 
        ld ix, tmp_mem 
        ld [ix+0], 0x10
        ld a, 0x10 
        cp [ix+0] 
        jp nz, is_fail 
        call is_pass

        ; --- Bloco 60: DAA Extensivo (525-540) ---
        
        ld de, msg_t525 
        call print 
        ld a, 0x15 
        add a, 0x27 
        daa 
        cp 0x42 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t526 
        call print 
        ld a, 0x88 
        add a, 0x11 
        daa 
        cp 0x99 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t527 
        call print 
        ld a, 0x99 
        add a, 0x01 
        daa 
        jp nc, is_fail 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t528 
        call print 
        ld a, 0x55 
        add a, 0x55 
        daa 
        jp nc, is_fail 
        cp 0x10 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t529 
        call print 
        ld a, 0x38 
        add a, 0x45 
        daa 
        cp 0x83 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t530 
        call print 
        ld a, 0x42 
        sub 0x15 
        daa 
        cp 0x27 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t531 
        call print 
        ld a, 0x10 
        sub 0x01 
        daa 
        cp 0x09 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t532 
        call print 
        ld a, 0x00 
        sub 0x01 
        daa 
        jp nc, is_fail 
        cp 0x99 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t533 
        call print 
        ld a, 0x83 
        sub 0x38 
        daa 
        cp 0x45 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t534 
        call print 
        scf 
        ld a, 0x10 
        adc a, 0x00 
        daa 
        cp 0x11 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t535 
        call print 
        scf 
        ld a, 0x99 
        adc a, 0x00 
        daa 
        jp nc, is_fail 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t536 
        call print 
        scf 
        ld a, 0x20 
        sbc a, 0x00 
        daa 
        cp 0x19 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t537 
        call print 
        scf 
        ld a, 0x00 
        sbc a, 0x00 
        daa 
        jp nc, is_fail 
        cp 0x99 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t538 
        call print 
        ld a, 0x09 
        inc a 
        daa 
        cp 0x10 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t539 
        call print 
        ld a, 0x10 
        dec a 
        daa 
        cp 0x09 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t540 
        call print 
        ld a, 0x99 
        inc a 
        daa 
        cp 0x00 
        jp nz, is_fail 
        call is_pass ; Note: INC doesn't set C

        ; --- Bloco 61: CPL, NEG, CCF, SCF, RLA, RRA (541-560) ---
        
        ld de, msg_t541 
        call print 
        ld a, 0x55 
        cpl 
        cp 0xAA 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t542 
        call print 
        ld a, 0x00 
        cpl 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t543 
        call print 
        ld a, 0x55 
        cpl 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; CPL H=1
        
        ld de, msg_t544 
        call print 
        ld a, 0x55 
        cpl 
        push af 
        pop hl 
        bit 1, l 
        jp z, is_fail 
        call is_pass ; CPL N=1
        
        ld de, msg_t545 
        call print 
        ld a, 0x01 
        neg 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t546 
        call print 
        ld a, 0x00 
        neg 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t547 
        call print 
        ld a, 0x80 
        neg 
        jp po, is_fail 
        cp 0x80 
        jp nz, is_fail 
        call is_pass ; NEG 80 -> V=1
        
        ld de, msg_t548 
        call print 
        ld a, 0x00 
        neg 
        jp c, is_fail 
        call is_pass ; NEG 00 -> C=0
        
        ld de, msg_t549 
        call print 
        ld a, 0x01 
        neg 
        jp nc, is_fail 
        call is_pass ; NEG != 00 -> C=1
        
        ld de, msg_t550 
        call print 
        or a 
        scf 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t551 
        call print 
        scf 
        ccf 
        jp c, is_fail 
        call is_pass
        
        ld de, msg_t552 
        call print 
        or a 
        ccf 
        jp nc, is_fail 
        call is_pass
        
        ld de, msg_t553 
        call print 
        scf 
        push af 
        pop hl 
        bit 4, l 
        jp nz, is_fail 
        call is_pass ; SCF H=0
        
        ld de, msg_t554 
        call print 
        scf 
        push af 
        pop hl 
        bit 1, l 
        jp nz, is_fail 
        call is_pass ; SCF N=0
        
        ld de, msg_t555 
        call print 
        scf 
        ccf 
        push af 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; CCF prev C=1 -> H=1
        
        ld de, msg_t556 
        call print 
        or a 
        ccf 
        push af 
        pop hl 
        bit 4, l 
        jp nz, is_fail 
        call is_pass ; CCF prev C=0 -> H=0
        
        ld de, msg_t557 
        call print 
        ld a, 0x80 
        or a 
        rla 
        jp nc, is_fail 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t558 
        call print 
        ld a, 0x01 
        or a 
        rra 
        jp nc, is_fail 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t559 
        call print 
        ld a, 0x80 
        or a 
        rlca 
        jp nc, is_fail 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t560 
        call print 
        ld a, 0x01 
        or a 
        rrca 
        jp nc, is_fail 
        cp 0x80 
        jp nz, is_fail 
        call is_pass

        ; --- Bloco 62: INC/DEC 16-bit e Extensões (561-580) ---
        
        ld de, msg_t561 
        call print 
        ld bc, 0x0000 
        inc bc 
        ld a, c 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t562 
        call print 
        ld bc, 0x00FF 
        inc bc 
        ld a, b 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t563 
        call print 
        ld bc, 0xFFFF 
        inc bc 
        ld a, b 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t564 
        call print 
        ld de, 0x0000 
        inc de 
        ld a, e 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t565 
        call print 
        ld de, 0x00FF 
        inc de 
        ld a, d 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t566 
        call print 
        ld de, 0xFFFF 
        inc de 
        ld a, d 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t567 
        call print 
        ld hl, 0x0000 
        inc hl 
        ld a, l 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t568 
        call print 
        ld hl, 0x00FF 
        inc hl 
        ld a, h 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t569 
        call print 
        ld hl, 0xFFFF 
        inc hl 
        ld a, h 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t570 
        call print 
        ld sp, 0x0000 
        inc sp 
        ld hl, 0x0000 
        add hl, sp 
        push hl 
        pop de 
        ld a, d 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t571 
        call print 
        ld bc, 0x0000 
        dec bc 
        ld a, c 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t572 
        call print 
        ld bc, 0x0100 
        dec bc 
        ld a, b 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t573 
        call print 
        ld de, 0x0000 
        dec de 
        ld a, e 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t574 
        call print 
        ld de, 0x0100 
        dec de 
        ld a, d 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t575 
        call print 
        ld hl, 0x0000 
        dec hl 
        ld a, l 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t576 
        call print 
        ld hl, 0x0100 
        dec hl 
        ld a, h 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t577 
        call print 
        ld ix, 0x0000 
        inc ix 
        push ix 
        pop de 
        ld a, e 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t578 
        call print 
        ld iy, 0x0000 
        inc iy 
        push iy 
        pop de 
        ld a, e 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t579 
        call print 
        ld ix, 0x0000 
        dec ix 
        push ix 
        pop de 
        ld a, e 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t580 
        call print 
        ld iy, 0x0000 
        dec iy 
        push iy 
        pop de 
        ld a, e 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass

        ; --- Bloco 63: INC/DEC 8-bit (581-600) ---
        
        ld de, msg_t581 
        call print 
        ld a, 0x00 
        inc a 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t582 
        call print 
        ld a, 0xFF 
        inc a 
        cp 0x00 
        jp nz, is_fail 
        jp nz, is_fail 
        call is_pass ; Z=1
        
        ld de, msg_t583 
        call print 
        ld a, 0x7F 
        inc a 
        jp po, is_fail 
        cp 0x80 
        jp nz, is_fail 
        call is_pass ; V=1
        
        ld de, msg_t584 
        call print 
        ld a, 0x0F 
        inc a 
        push af 
        cp 0x10 
        jp nz, is_fail 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; H=1
        
        ld de, msg_t585 
        call print 
        ld b, 0x00 
        inc b 
        ld a, b 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t586 
        call print 
        ld c, 0xFF 
        inc c 
        ld a, c 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t587 
        call print 
        ld d, 0x7F 
        inc d 
        ld a, d 
        jp po, is_fail 
        cp 0x80 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t588 
        call print 
        ld e, 0x0F 
        inc e 
        ld a, e 
        push af 
        cp 0x10 
        jp nz, is_fail 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t589 
        call print 
        ld h, 0x00 
        inc h 
        ld a, h 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t590 
        call print 
        ld l, 0xFF 
        inc l 
        ld a, l 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t591 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x00 
        inc [hl] 
        ld a, [hl] 
        cp 0x01 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t592 
        call print 
        ld a, 0x01 
        dec a 
        cp 0x00 
        jp nz, is_fail 
        jp nz, is_fail 
        call is_pass ; Z=1
        
        ld de, msg_t593 
        call print 
        ld a, 0x00 
        dec a 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t594 
        call print 
        ld a, 0x80 
        dec a 
        jp po, is_fail 
        cp 0x7F 
        jp nz, is_fail 
        call is_pass ; V=1
        
        ld de, msg_t595 
        call print 
        ld a, 0x10 
        dec a 
        push af 
        cp 0x0F 
        jp nz, is_fail 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass ; H=1
        
        ld de, msg_t596 
        call print 
        ld b, 0x01 
        dec b 
        ld a, b 
        cp 0x00 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t597 
        call print 
        ld c, 0x00 
        dec c 
        ld a, c 
        cp 0xFF 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t598 
        call print 
        ld d, 0x80 
        dec d 
        jp po, is_fail 
        ld a, d 
        cp 0x7F 
        jp nz, is_fail 
        call is_pass
        
        ld de, msg_t599 
        call print 
        ld e, 0x10 
        dec e 
        push af 
        ld a, e 
        cp 0x0F 
        jp nz, is_fail 
        pop hl 
        bit 4, l 
        jp z, is_fail 
        call is_pass
        
        ld de, msg_t600 
        call print 
        ld hl, tmp_mem 
        ld [hl], 0x01 
        dec [hl] 
        ld a, [hl] 
        cp 0x00 
        jp nz, is_fail 
        call is_pass

_stage2:
        ld de, msg_t601 
        call print 
        call reset_flags
        ld a, 0xff
        neg
        ld de, 0x0113
        call print_flags

        ld de, msg_t602
        call print 
        call reset_flags
        ld a, 0x00
        neg
        ld de, 0x0042
        call print_flags

        ld de, msg_t603
        call print 
        call reset_flags
        ld a, 0x55
        neg
        ld de, 0xab93
        call print_flags

        ld de, msg_t604
        call print 
        call reset_flags
        ld a, 0xff
        bit 0, a
        ld de, 0xff10
        call print_flags

        ld de, msg_t605
        call print 
        call reset_flags
        ld a, 0x00
        bit 0, a
        ld de, 0x0054
        call print_flags

        ld de, msg_t606
        call print 
        call reset_flags
        ld a, 0xff
        bit 1, a
        ld de, 0xff10
        call print_flags

        ld de, msg_t607
        call print 
        call reset_flags
        ld a, 0x00
        bit 1, a
        ld de, 0x0054
        call print_flags

        ld de, msg_t608
        call print 
        call reset_flags
        ld a, 0xff
        bit 2, a
        ld de, 0xff10
        call print_flags

        ld de, msg_t609
        call print 
        call reset_flags
        ld a, 0x00
        bit 2, a
        ld de, 0x0054
        call print_flags

        ld de, msg_t610
        call print 
        call reset_flags
        ld a, 0xff
        bit 3, a
        ld de, 0xff10
        call print_flags

        ld de, msg_t611
        call print 
        call reset_flags
        ld a, 0x00
        bit 3, a
        ld de, 0x0054
        call print_flags

        ld de, msg_t612
        call print 
        call reset_flags
        ld a, 0xff
        bit 4, a
        ld de, 0xff10
        call print_flags

        ld de, msg_t613
        call print 
        call reset_flags
        ld a, 0x00
        bit 4, a
        ld de, 0x0054
        call print_flags

        ld de, msg_t614
        call print 
        call reset_flags
        ld a, 0xff
        bit 5, a
        ld de, 0xff10
        call print_flags

        ld de, msg_t615
        call print 
        call reset_flags
        ld a, 0x00
        bit 5, a
        ld de, 0x0054
        call print_flags

        ld de, msg_t616
        call print 
        call reset_flags
        ld a, 0xff
        bit 6, a
        ld de, 0xff10
        call print_flags

        ld de, msg_t617
        call print 
        call reset_flags
        ld a, 0x00
        bit 6, a
        ld de, 0x0054
        call print_flags

        ld de, msg_t618
        call print 
        call reset_flags
        ld a, 0xff
        bit 7, a
        ld de, 0xff90
        call print_flags

        ld de, msg_t619
        call print 
        call reset_flags
        ld a, 0x00
        bit 7, a
        ld de, 0x0054
        call print_flags

        ld de, msg_t620
        call print 
        call reset_flags
        scf
        ld a, 0x00
        rla
        ld de, 0x0100
        call print_flags

        ld de, msg_t621
        call print 
        call reset_flags
        scf
        ld a, 0x00
        rl a
        ld de, 0x0100
        call print_flags

        ld de, msg_t622
        call print 
        call reset_flags
        scf
        ld a, 0x00
        rra
        ld de, 0x8000
        call print_flags

        ld de, msg_t623
        call print 
        call reset_flags
        scf
        ld a, 0x00
        rr a
        ld de, 0x8080
        call print_flags

        ld de, msg_t624
        call print 
        call reset_flags
        scf
        ld a, 0xaa
        xor a
        ld de, 0x0044
        call print_flags

        ld de, msg_t625
        call print 
        call reset_flags
        scf
        ld a, 0x55
        xor a
        ld de, 0x0044
        call print_flags

        ld de, msg_t626
        call print 
        call reset_flags
        ld a, 0x80
        sub 0xff
        ld de, 0x8193
        call print_flags

        ld de, msg_t627
        call print 
        call reset_flags
        ld hl, tmp_mem 
        ld [hl], 0xff 
        ld a, 0x00 
        ld bc, 0x0001 
        cpi 
        ld a, [hl]
        ld de, 0x0012
        call print_flags

        ld de, msg_t628
        call print 
        call reset_flags
        ld a, 0x20 
        cp 0x10
        ld de, 0x2002
        call print_flags

        ld de, msg_t629
        call print
        ld hl, 0x0040
        push hl
        pop af
        call z, is_pass
        jp nz, is_fail

        ld de, msg_t630
        call print
        ld hl, 0x0000
        push hl
        pop af
        call nz, is_pass
        jp z, is_fail

        ld de, msg_t631
        call print
        ld hl, 0x0080
        push hl
        pop af
        call m, is_pass
        jp p, is_fail

        ld de, msg_t632
        call print
        ld hl, 0x0000
        push hl
        pop af
        call p, is_pass
        jp m, is_fail

        ld de, msg_t633
        call print
        ld hl, 0x0001
        push hl
        pop af
        call c, is_pass
        jp nc, is_fail

        ld de, msg_t634
        call print
        ld hl, 0x0000
        push hl
        pop af
        call nc, is_pass
        jp c, is_fail

        ld de, msg_t635
        call print
        ld hl, 0x0004
        push hl
        pop af
        call pe, is_pass
        jp po, is_fail

        ld de, msg_t636
        call print
        ld hl, 0x0000
        push hl
        pop af
        call po, is_pass
        jp pe, is_fail

        ld de, msg_t637
        call print 
        call reset_flags
        ld a, 0x10 
        cp 0x20
        ld de, 0x1083
        call print_flags

        ld de, msg_t638
        call print 
        call reset_flags
        ld a, 0x10 
        cp 0x10
        ld de, 0x1042
        call print_flags

        ld de, msg_t639
        call print 
        call reset_flags
        ld a, 0x10 
        cp 0x80
        ld de, 0x1087
        call print_flags

        ld de, msg_t640
        call print 
        call reset_flags
        ld a, 0x80 
        cp 0x10
        ld de, 0x8006
        call print_flags

        ld de, msg_t641
        call print 
        call reset_flags
        ld a, 0x90 
        and 0x10
        ld de, 0x1010
        call print_flags

        ld de, msg_t642
        call print 
        call reset_flags
        ld a, 0xaa 
        and 0x55
        ld de, 0x0054
        call print_flags

        ld de, msg_t643
        call print 
        call reset_flags
        ld a, 0xff
        and 0xff
        ld de, 0xff94
        call print_flags

        ld de, msg_t644
        call print 
        call reset_flags
        ld a, 0x90 
        xor 0x10
        ld de, 0x8080
        call print_flags

        ld de, msg_t645
        call print 
        call reset_flags
        ld a, 0xaa 
        xor 0x55
        ld de, 0xff84
        call print_flags

        ld de, msg_t646
        call print 
        call reset_flags
        ld a, 0xff
        xor 0xff
        ld de, 0x0044
        call print_flags

        ld de, msg_t647
        call print 
        call reset_flags
        ld a, 0x90 
        or 0x10
        ld de, 0x9084
        call print_flags

        ld de, msg_t648
        call print 
        call reset_flags
        ld a, 0xaa 
        or 0x55
        ld de, 0xff84
        call print_flags

        ld de, msg_t649
        call print 
        call reset_flags
        ld a, 0xff
        or 0xff
        ld de, 0xff84
        call print_flags

        ld de, msg_t650
        call print 
        call reset_flags
        ld a, 0x10 
        sub a, 0x20
        ld de, 0xf083
        call print_flags

        ld de, msg_t651
        call print 
        call reset_flags
        ld a, 0x10 
        sub a, 0x10
        ld de, 0x0042
        call print_flags

        ld de, msg_t652
        call print 
        call reset_flags
        ld a, 0x10 
        sub a, 0x80
        ld de, 0x9087
        call print_flags

        ld de, msg_t653
        call print 
        call reset_flags
        ld a, 0x80 
        sub a, 0x10
        ld de, 0x7006
        call print_flags

        ld de, msg_t654
        call print 
        call reset_flags
        ld a, 0x10
        scf
        sbc a, 0x20
        ld de, 0xef93
        call print_flags

        ld de, msg_t655
        call print 
        call reset_flags
        ld a, 0x10 
        scf
        sbc a, 0x10
        ld de, 0xff93
        call print_flags

        ld de, msg_t656
        call print 
        call reset_flags
        ld a, 0x10 
        scf
        sbc a, 0x80
        ld de, 0x8f97
        call print_flags

        ld de, msg_t657
        call print 
        call reset_flags
        ld a, 0x80 
        scf
        sbc a, 0x10
        ld de, 0x6f16
        call print_flags

        ld de, msg_t658
        call print 
        call reset_flags
        ld a, 0x10
        scf
        adc a, 0x20
        ld de, 0x3100
        call print_flags

        ld de, msg_t659
        call print 
        call reset_flags
        ld a, 0x10 
        scf
        adc a, 0x10
        ld de, 0x2100
        call print_flags

        ld de, msg_t660
        call print 
        call reset_flags
        ld a, 0x10 
        scf
        adc a, 0x80
        ld de, 0x9180
        call print_flags

        ld de, msg_t661
        call print 
        call reset_flags
        ld a, 0x80 
        scf
        adc a, 0x10
        ld de, 0x9180
        call print_flags

        ld de, msg_t662
        call print 
        call reset_flags
        ld a, 0xff 
        scf
        adc a, 0x01
        ld de, 0x0111
        call print_flags

        ld de, msg_t663
        call print 
        call reset_flags
        ld a, 0x00 
        scf
        sbc a, 0x01
        ld de, 0xfe93
        call print_flags

        ld de, msg_t664
        call print 
        call reset_flags
        ld a, 0x7f 
        add a, 0x01
        ld de, 0x8094
        call print_flags

        ld de, msg_t664
        call print 
        call reset_flags
        ld a, 0xff 
        scf
        adc a, 0x00
        ld de, 0x0051
        call print_flags

        ld de, msg_t665
        call print
        call reset_flags
        ld a, 0x99
        add a, 1
        daa
        ld de, 0x0055
        call print_flags

        ld de, msg_t666
        call print
        call reset_flags
        ld a, 0x50
        sub a, 0x25
        daa
        ld de, 0x2502
        call print_flags

        ld de, msg_t667
        call print
        call reset_flags
        ld a, 0xff
        add a, 0x00
        daa
        ld de, 0x6515
        call print_flags
        
        ld de, msg_t668
        call print
        call reset_flags
        ld hl, tmp_mem
        ld a, 0x00
        ld [hl], a
        ld bc, 1
        cpi
        ld de, 0x0042
        call print_flags

        ld de, msg_t669
        call print
        call reset_flags
        ld hl, tmp_mem
        ld a, 0x00
        ld [hl], a
        inc hl
        inc a
        ld [hl], a
        inc hl
        inc a
        ld [hl], a
        inc hl
        inc a
        ld hl, tmp_mem
        ld bc, 4
        ld a, 0
        cpi
        ld de, 0x0046
        call print_flags

        ld de, msg_t670
        call print
        call reset_flags
        ld hl, tmp_mem
        ld a, 0x00
        ld [hl], a
        inc hl
        inc a
        ld [hl], a
        inc hl
        inc a
        ld [hl], a
        inc hl
        inc a
        ld hl, tmp_mem
        ld bc, 4
        ld a, 0
        cpir
        push af
        push hl
        ld de, tmp_mem
        or a
        sbc hl, de
        ld a, h
        call print_hex_byte
        ld a, l
        call print_hex_byte
        ld de, 0x0046
        pop hl
        pop af
        call print_flags

        ld de, msg_t671
        call print
        call reset_flags
        ld hl, tmp_mem
        ld a, 0x00
        ld [hl], a
        inc hl
        inc a
        ld [hl], a
        inc hl
        inc a
        ld [hl], a
        inc a
        ld bc, 4
        ld a, 0
        cpir
        push af
        push hl
        ld de, tmp_mem
        or a
        sbc hl, de
        ld a, h
        call print_hex_byte
        ld a, l
        call print_hex_byte
        ld de, 0x0046
        pop hl
        pop af
        call print_flags
        
        ld de, msg_t672
        call print
        call reset_flags
        ld hl, tmp_mem
        ld [hl], 0x05
        inc hl
        ld [hl], 0x10
        ld hl, tmp_mem
        ld bc, 2
        ld a, 0x10
        cpir
        push af
        push hl
        ld de, tmp_mem
        or a
        sbc hl, de
        ld a, h
        call print_hex_byte
        ld a, l
        call print_hex_byte
        ld de, 0x1042
        pop hl
        pop af
        call print_flags
        


        ret                 

; ------------------------------------------------------------
; area de dados
; ------------------------------------------------------------
section data

tmp_mem: ds 16 

; -- bloco 1 a 14 strings omitidas em modo full text, mas inseridas para completude --
msg_t01: db "001: ADD A,0 (A=0) $"
msg_t02: db "002: ADD A,1 (A=FF) $"
msg_t03: db "003: ADD A,1 (A=7F) $"
msg_t04: db "004: ADD A,1 (A=0F) $"
msg_t05: db "005: ADD A,80 (A=80) $"
msg_t06: db "006: ADC A,0 (A=0,C=1) $"
msg_t07: db "007: ADC A,0 (A=FF,C=1) $"
msg_t08: db "008: ADC A,0 (A=7F,C=1) $"
msg_t09: db "009: ADC A,0 (A=0,C=0) $"
msg_t10: db "010: ADC A,0 (A=0F,C=1) $"
msg_t11: db "011: SUB 0 (A=0) $"
msg_t12: db "012: SUB 1 (A=0) $"
msg_t13: db "013: SUB 80 (A=7F) $"
msg_t14: db "014: SUB 1 (A=10) $"
msg_t15: db "015: SUB A (A=55) $"
msg_t16: db "016: SBC A,0 (A=0,C=1) $"
msg_t17: db "017: SBC A,5 (A=5,C=0) $"
msg_t18: db "018: SBC A,0 (A=FF,C=1) $"
msg_t19: db "019: SBC A,1 (A=2,C=1) $"
msg_t20: db "020: SBC A,A (A=55,C=1) $"
msg_t21: db "021: CP 0 (A=0) $"
msg_t22: db "022: CP 1 (A=0) $"
msg_t23: db "023: CP FF (A=FE) $"
msg_t24: db "024: CP 80 (A=7F) $"
msg_t25: db "025: CP 10 (A=20) $"
msg_t26: db "026: AND 0 (A=FF) $"
msg_t27: db "027: AND A (A=55) $"
msg_t28: db "028: AND 55 (A=AA) $"
msg_t29: db "029: AND 80 (A=80) $"
msg_t30: db "030: AND 0F (A=FF) $"
msg_t31: db "031: OR 0 (A=0) $"
msg_t32: db "032: OR FF (A=0) $"
msg_t33: db "033: OR A (A=55) $"
msg_t34: db "034: OR 80 (A=1) $"
msg_t35: db "035: OR 0 (A=7F) $"
msg_t36: db "036: XOR A $"
msg_t37: db "037: XOR FF (A=FF) $"
msg_t38: db "038: XOR 55 (A=AA) $"
msg_t39: db "039: XOR 80 (A=7F) $"
msg_t40: db "040: XOR 1 (A=1) $"
msg_t41: db "041: INC A (A=FF) $"
msg_t42: db "042: INC A (A=7F) $"
msg_t43: db "043: INC A (A=0F) $"
msg_t44: db "044: INC A (A=0) $"
msg_t45: db "045: INC A (A=FE) $"
msg_t46: db "046: DEC A (A=0) $"
msg_t47: db "047: DEC A (A=80) $"
msg_t48: db "048: DEC A (A=1) $"
msg_t49: db "049: DEC A (A=10) $"
msg_t50: db "050: DEC A (A=2) $"
msg_t51: db "051: SLA A (A=80) $"
msg_t52: db "052: SRA A (A=81) $"
msg_t53: db "053: SRL A (A=1) $"
msg_t54: db "054: SLA A (A=FF) $"
msg_t55: db "055: SRA A (A=8) $"
msg_t56: db "056: RLC A (A=80) $"
msg_t57: db "057: RRC A (A=1) $"
msg_t58: db "058: RL A (A=0,C=1) $"
msg_t59: db "059: RR A (A=0,C=1) $"
msg_t60: db "060: RLC A (A=FF) $"
msg_t61: db "061: RLCA (A=80) $"
msg_t62: db "062: RRCA (A=1) $"
msg_t63: db "063: RLA (A=0,C=1) $"
msg_t64: db "064: RRA (A=0,C=1) $"
msg_t65: db "065: SCF $"
msg_t66: db "066: CCF $"
msg_t67: db "067: CPL $"
msg_t68: db "068: NEG $"
msg_t69: db "069: BIT 7,A $"
msg_t70: db "070: BIT 0,A $"
; -- novos 70 testes (16-bit) --
msg_t71: db "071: ADD HL,BC (Preserva Z=0) $"
msg_t72: db "072: ADD HL,DE (Preserva Z=1) $"
msg_t73: db "073: ADD HL,HL (C=0) $"
msg_t74: db "074: ADD HL,BC (C=1, H=1) $"
msg_t75: db "075: ADD HL,DE (Normal) $"
msg_t76: db "076: ADC HL,BC (Afeta Z=0) $"
msg_t77: db "077: ADC HL,DE (Afeta Z=1) $"
msg_t78: db "078: ADC HL,BC (S=1, V=1) $"
msg_t79: db "079: ADC HL,DE (Sem Carry) $"
msg_t80: db "080: ADC HL,BC (H=1) $"
msg_t81: db "081: SBC HL,BC (Z=1) $"
msg_t82: db "082: SBC HL,DE (C=1, S=1) $"
msg_t83: db "083: SBC HL,BC (V=1) $"
msg_t84: db "084: SBC HL,DE (H=1, Borrow) $"
msg_t85: db "085: SBC HL,BC (Normal Z=1) $"
msg_t86: db "086: INC BC (Preserva C=1) $"
msg_t87: db "087: DEC DE (Preserva C=0) $"
msg_t88: db "088: INC HL (Preserva Z=1) $"
msg_t89: db "089: DEC BC (Preserva Z=0) $"
msg_t90: db "090: INC/DEC HL $"
msg_t91: db "091: DAA Add (S/ Carry) $"
msg_t92: db "092: DAA Add (C/ Half-C) $"
msg_t93: db "093: DAA Add (C/ Carry) $"
msg_t94: db "094: DAA Add (C/ Both) $"
msg_t95: db "095: DAA Add (Edge) $"
msg_t96: db "096: DAA Sub (S/ Carry) $"
msg_t97: db "097: DAA Sub (C/ Half-C) $"
msg_t98: db "098: DAA Sub (C/ Carry) $"
msg_t99: db "099: DAA Sub (C/ Both) $"
msg_t100: db "100: DAA Sub (Edge) $"
msg_t101: db "101: RLD (Normal) $"
msg_t102: db "102: RLD (Z=1) $"
msg_t103: db "103: RLD (S=0) $"
msg_t104: db "104: RRD (Normal) $"
msg_t105: db "105: RRD (Edge) $"
msg_t106: db "106: NEG (80h) $"
msg_t107: db "107: NEG (00h) $"
msg_t108: db "108: CPL (H=1, N=1) $"
msg_t109: db "109: CCF (De C=1) $"
msg_t110: db "110: CCF (De C=0) $"
msg_t111: db "111: ADD IX,BC $"
msg_t112: db "112: ADD IY,DE $"
msg_t113: db "113: INC/DEC IX $"
msg_t114: db "114: ADD IY,BC $"
msg_t115: db "115: ADD IX,IX $"
msg_t116: db "116: SET 7,A $"
msg_t117: db "117: RES 7,A $"
msg_t118: db "118: SET 0,A $"
msg_t119: db "119: RES 0,B $"
msg_t120: db "120: SET 4,C $"
msg_t121: db "121: INC [hl] (Zero) $"
msg_t122: db "122: DEC [hl] (Minus) $"
msg_t123: db "123: INC [hl] (V=1) $"
msg_t124: db "124: DEC [hl] (V=1) $"
msg_t125: db "125: INC [hl] (Pres C) $"
msg_t126: db "126: SLA [hl] $"
msg_t127: db "127: SRA [hl] $"
msg_t128: db "128: SRL [hl] $"
msg_t129: db "129: RL [hl] $"
msg_t130: db "130: RR [hl] $"
msg_t131: db "131: ADD FF+FF $"
msg_t132: db "132: ADC 00+FF+C $"
msg_t133: db "133: SBC 00-FF $"
msg_t134: db "134: SUB 80-FF $"
msg_t135: db "135: AND 55&AA $"
msg_t136: db "136: OR 55|AA $"
msg_t137: db "137: XOR FF^0F $"
msg_t138: db "138: ADD HLx4 $"
msg_t139: db "139: ADC HL,HL+C $"
msg_t140: db "140: SBC HL,HL+C $"
msg_t141: db "141: BIT 0, A $"
msg_t142: db "142: BIT 1, A $"
msg_t143: db "143: BIT 2, A $"
msg_t144: db "144: BIT 3, A $"
msg_t145: db "145: BIT 4, A $"
msg_t146: db "146: BIT 5, A $"
msg_t147: db "147: BIT 6, A $"
msg_t148: db "148: BIT 7, A $"
msg_t149: db "149: BIT 0, B $"
msg_t150: db "150: BIT 1, B $"
msg_t151: db "151: BIT 2, B $"
msg_t152: db "152: BIT 3, B $"
msg_t153: db "153: BIT 4, B $"
msg_t154: db "154: BIT 5, B $"
msg_t155: db "155: BIT 6, B $"
msg_t156: db "156: BIT 7, B $"
msg_t157: db "157: BIT 0, C $"
msg_t158: db "158: BIT 1, C $"
msg_t159: db "159: BIT 2, C $"
msg_t160: db "160: BIT 3, C $"
msg_t161: db "161: BIT 4, C $"
msg_t162: db "162: BIT 5, C $"
msg_t163: db "163: BIT 6, C $"
msg_t164: db "164: BIT 7, C $"
msg_t165: db "165: BIT 0, D $"
msg_t166: db "166: BIT 1, D $"
msg_t167: db "167: BIT 2, D $"
msg_t168: db "168: BIT 3, D $"
msg_t169: db "169: BIT 4, D $"
msg_t170: db "170: BIT 5, D $"
msg_t171: db "171: BIT 6, D $"
msg_t172: db "172: BIT 7, D $"
msg_t173: db "173: SET 0, A $"
msg_t174: db "174: SET 1, A $"
msg_t175: db "175: SET 2, A $"
msg_t176: db "176: SET 3, A $"
msg_t177: db "177: SET 4, A $"
msg_t178: db "178: SET 5, A $"
msg_t179: db "179: SET 6, A $"
msg_t180: db "180: SET 7, A $"
msg_t181: db "181: SET 0, B $"
msg_t182: db "182: SET 1, B $"
msg_t183: db "183: SET 2, B $"
msg_t184: db "184: SET 3, B $"
msg_t185: db "185: SET 4, B $"
msg_t186: db "186: SET 5, B $"
msg_t187: db "187: SET 6, B $"
msg_t188: db "188: SET 7, B $"
msg_t189: db "189: SET 0, C $"
msg_t190: db "190: SET 1, C $"
msg_t191: db "191: SET 2, C $"
msg_t192: db "192: SET 3, C $"
msg_t193: db "193: SET 4, C $"
msg_t194: db "194: SET 5, C $"
msg_t195: db "195: SET 6, C $"
msg_t196: db "196: SET 7, C $"
msg_t197: db "197: RES 0, A $"
msg_t198: db "198: RES 1, A $"
msg_t199: db "199: RES 2, A $"
msg_t200: db "200: RES 3, A $"
msg_t201: db "201: RES 4, A $"
msg_t202: db "202: RES 5, A $"
msg_t203: db "203: RES 6, A $"
msg_t204: db "204: RES 7, A $"
msg_t205: db "205: RES 0, B $"
msg_t206: db "206: RES 1, B $"
msg_t207: db "207: RES 2, B $"
msg_t208: db "208: RES 3, B $"
msg_t209: db "209: RES 4, B $"
msg_t210: db "210: RES 5, B $"
msg_t211: db "211: RES 6, B $"
msg_t212: db "212: RES 7, B $"
msg_t213: db "213: RES 0, C $"
msg_t214: db "214: RES 1, C $"
msg_t215: db "215: RES 2, C $"
msg_t216: db "216: RES 3, C $"
msg_t217: db "217: RES 4, C $"
msg_t218: db "218: RES 5, C $"
msg_t219: db "219: RES 6, C $"
msg_t220: db "220: RES 7, C $"
msg_t221: db "221: POP AF (Z=0) $"
msg_t222: db "222: POP AF (C=0) $"
msg_t223: db "223: POP AF (S=1) $"
msg_t224: db "224: POP AF (C=1) $"
msg_t225: db "225: POP AF (P/V=1) $"
msg_t226: db "226: POP AF (Z=1) $"
msg_t227: db "227: POP AF Test Z $"
msg_t228: db "228: POP AF Test S $"
msg_t229: db "229: POP AF Test C $"
msg_t230: db "230: POP AF Test P/V $"
msg_t231: db "231: POP AF Test H $"
msg_t232: db "232: POP AF Test N $"
msg_t233: db "233: PUSH/POP A (55) $"
msg_t234: db "234: PUSH/POP A (AA) $"
msg_t235: db "235: PUSH/POP AF (C=0) $"
msg_t236: db "236: PUSH/POP AF (C=1) $"
msg_t237: db "237: PUSH/POP AF (S=0) $"
msg_t238: db "238: PUSH/POP AF (Z=1) $"
msg_t239: db "239: PUSH/POP AF (H=1) $"
msg_t240: db "240: PUSH/POP AF (N=1) $"
msg_t241: db "241: LDI P/V=0 $"
msg_t242: db "242: LDI P/V=1 $"
msg_t243: db "243: LDI H=0 $"
msg_t244: db "244: LDI N=0 $"
msg_t245: db "245: LDD P/V=0 $"
msg_t246: db "246: LDD P/V=1 $"
msg_t247: db "247: CPI Z=1 $"
msg_t248: db "248: CPI Z=0 $"
msg_t249: db "249: CPI P/V=0 $"
msg_t250: db "250: CPI P/V=1 $"
msg_t251: db "251: CPD N=1 $"
msg_t252: db "252: CPD Z=1 $"
msg_t253: db "253: CPD Z=0 $"
msg_t254: db "254: CPD P/V=0 $"
msg_t255: db "255: CPD P/V=1 $"
msg_t256: db "256: CPI S=1 $"
msg_t257: db "257: CPI H=1 $"
msg_t258: db "258: CPD S=1 $"
msg_t259: db "259: CPD H=1 $"
msg_t260: db "260: LDI Y=0 $"
msg_t261: db "261: ADD IX,BC $"
msg_t262: db "262: ADD IX,DE $"
msg_t263: db "263: ADD IX,SP $"
msg_t264: db "264: ADD IY,BC $"
msg_t265: db "265: ADD IY,DE $"
msg_t266: db "266: ADD IY,SP $"
msg_t267: db "267: ADD IX C=1 $"
msg_t268: db "268: ADD IY C=1 $"
msg_t269: db "269: ADD IX H=1 $"
msg_t270: db "270: ADD IY H=1 $"
msg_t271: db "271: INC IX $"
msg_t272: db "272: DEC IY $"
msg_t273: db "273: PUSH IX/POP DE $"
msg_t274: db "274: LD [ix+d] $"
msg_t275: db "275: LD [iy+d] $"
msg_t276: db "276: INC [ix+d] Z $"
msg_t277: db "277: DEC [iy+d] S $"
msg_t278: db "278: INC [ix+d] V $"
msg_t279: db "279: DEC [iy+d] V $"
msg_t280: db "280: INC [ix+d] Pres C $"
msg_t281: db "281: SLA [hl] S=1 $"
msg_t282: db "282: SLA [hl] Z=0 $"
msg_t283: db "283: SRA [hl] S=1 $"
msg_t284: db "284: SRA [hl] C=1 $"
msg_t285: db "285: SRL [hl] C=1 $"
msg_t286: db "286: SRL [hl] Z=1 $"
msg_t287: db "287: RL [hl] C=0 $"
msg_t288: db "288: RL [hl] C=1 $"
msg_t289: db "289: RR [hl] C=0 $"
msg_t290: db "290: RR [hl] C=1 $"
msg_t291: db "291: RLC [hl] C=1 $"
msg_t292: db "292: RRC [hl] C=1 $"
msg_t293: db "293: SLA [ix+d] $"
msg_t294: db "294: SRA [iy+d] $"
msg_t295: db "295: SRL [ix+d] $"
msg_t296: db "296: RL [iy+d] $"
msg_t297: db "297: RR [ix+d] $"
msg_t298: db "298: RLC [iy+d] $"
msg_t299: db "299: RRC [ix+d] $"
msg_t300: db "300: SET 7,[hl] $"
msg_t301: db "301: RES 7,[hl] $"
msg_t302: db "302: SET 3,[ix+d] $"
msg_t303: db "303: RES 3,[iy+d] $"
msg_t304: db "304: ADD H=1 $"
msg_t305: db "305: SUB H=1 $"
msg_t306: db "306: ADC H=1 $"
msg_t307: db "307: SBC H=1 $"
msg_t308: db "308: INC A H=1 $"
msg_t309: db "309: DEC B H=1 $"
msg_t310: db "310: INC C H=1 $"
msg_t311: db "311: DEC D H=1 $"
msg_t312: db "312: INC E H=1 $"
msg_t313: db "313: DEC H H=1 $"
msg_t314: db "314: INC L H=1 $"
msg_t315: db "315: CP H=1 $"
msg_t316: db "316: NEG 0 H=0 $"
msg_t317: db "317: NEG 1 H=1 $"
msg_t318: db "318: AND H=1 $"
msg_t319: db "319: OR H=0 $"
msg_t320: db "320: XOR H=0 $"
msg_t321: db "321: PUSH/POP BC $"
msg_t322: db "322: PUSH/POP DE $"
msg_t323: db "323: PUSH/POP HL $"
msg_t324: db "324: PUSH/POP IX $"
msg_t325: db "325: PUSH/POP IY $"
msg_t326: db "326: Flags SC Pres $"
msg_t327: db "327: Flags CC Pres $"
msg_t328: db "328: DAA 99+00 $"
msg_t329: db "329: DAA 00-99 $"
msg_t330: db "330: DAA 01-02 $"
msg_t331: db "331: DAA 50+50 $"
msg_t332: db "332: DAA 10-20 $"
msg_t333: db "333: CCF+CCF C=1 $"
msg_t334: db "334: CCF+CCF C=0 $"
msg_t335: db "335: INC BC OVF $"
msg_t336: db "336: DEC DE UNF $"
msg_t337: db "337: INC HL OVF $"
msg_t338: db "338: INC SP OVF $"
msg_t339: db "339: SBC 0,0+C DAA $"
msg_t340: db "340: ADC 0,0 DAA $"
msg_t341: db "341: DJNZ B=0 $"
msg_t342: db "342: DJNZ B=1 $"
msg_t343: db "343: INC C Z $"
msg_t344: db "344: DEC C S $"
msg_t345: db "345: EX AF,AF' $"
msg_t346: db "346: EXX $"
msg_t347: db "347: DEC [hl] S $"
msg_t348: db "348: INC [hl] Z $"
msg_t349: db "349: DEC [ix+d] S $"
msg_t350: db "350: INC [iy+d] Z $"
msg_t351: db "351: RLA/RRA FF $"
msg_t352: db "352: RLA/RRA 00 $"
msg_t353: db "353: RLA/RRA C=1 $"
msg_t354: db "354: RLA/RRA C=0 $"
msg_t355: db "355: RLD/RRD FF $"
msg_t356: db "356: RRD/RLD 00 $"
msg_t357: db "357: ADD A,B $"
msg_t358: db "358: ADD A,C $"
msg_t359: db "359: ADD A,D $"
msg_t360: db "360: ADD A,E $"
msg_t361: db "361: ADD A,H $"
msg_t362: db "362: ADD A,L $"
msg_t363: db "363: ADD A,A $"
msg_t364: db "364: ADD A,[hl] $"
msg_t365: db "365: ADC A,B $"
msg_t366: db "366: ADC A,C $"
msg_t367: db "367: ADC A,D $"
msg_t368: db "368: ADC A,E $"
msg_t369: db "369: ADC A,H $"
msg_t370: db "370: ADC A,L $"
msg_t371: db "371: ADC A,A $"
msg_t372: db "372: ADC A,[hl] $"
msg_t373: db "373: SUB B $"
msg_t374: db "374: SUB C $"
msg_t375: db "375: SUB D $"
msg_t376: db "376: SUB E $"
msg_t377: db "377: SUB H $"
msg_t378: db "378: SUB L $"
msg_t379: db "379: SUB A $"
msg_t380: db "380: SUB [hl] $"
msg_t381: db "381: SBC A,B $"
msg_t382: db "382: SBC A,C $"
msg_t383: db "383: SBC A,D $"
msg_t384: db "384: SBC A,E $"
msg_t385: db "385: SBC A,H $"
msg_t386: db "386: SBC A,L $"
msg_t387: db "387: SBC A,A $"
msg_t388: db "388: SBC A,[hl] $"
msg_t389: db "389: AND B $"
msg_t390: db "390: AND C $"
msg_t391: db "391: AND D $"
msg_t392: db "392: AND E $"
msg_t393: db "393: AND H $"
msg_t394: db "394: AND L $"
msg_t395: db "395: AND A $"
msg_t396: db "396: AND [hl] $"
msg_t397: db "397: XOR B $"
msg_t398: db "398: XOR C $"
msg_t399: db "399: XOR D $"
msg_t400: db "400: XOR E $"
msg_t401: db "401: XOR H $"
msg_t402: db "402: XOR L $"
msg_t403: db "403: XOR A $"
msg_t404: db "404: XOR [hl] $"
msg_t405: db "405: OR B $"
msg_t406: db "406: OR C $"
msg_t407: db "407: OR D $"
msg_t408: db "408: OR E $"
msg_t409: db "409: OR H $"
msg_t410: db "410: OR L $"
msg_t411: db "411: OR A $"
msg_t412: db "412: OR [hl] $"
msg_t413: db "413: CP B $"
msg_t414: db "414: CP C $"
msg_t415: db "415: CP D $"
msg_t416: db "416: CP E $"
msg_t417: db "417: CP H $"
msg_t418: db "418: CP L $"
msg_t419: db "419: CP A $"
msg_t420: db "420: CP [hl] $"
msg_t421: db "421: ADD n 00 $"
msg_t422: db "422: ADD n FF $"
msg_t423: db "423: ADD n H=1 $"
msg_t424: db "424: ADD n OVF 1 $"
msg_t425: db "425: ADD n OVF 2 $"
msg_t426: db "426: ADD n S=1 $"
msg_t427: db "427: ADD n C=1 $"
msg_t428: db "428: ADD n 7F+7F $"
msg_t429: db "429: ADC n 00 $"
msg_t430: db "430: ADC n FF $"
msg_t431: db "431: ADC n C=1 $"
msg_t432: db "432: ADC n H=1 $"
msg_t433: db "433: ADC n OVF 1 $"
msg_t434: db "434: ADC n OVF 2 $"
msg_t435: db "435: ADC n OVF 3 $"
msg_t436: db "436: ADC n 7F+7F $"
msg_t437: db "437: SUB n 00 $"
msg_t438: db "438: SUB n 01 $"
msg_t439: db "439: SUB n H=1 $"
msg_t440: db "440: SUB n OVF 1 $"
msg_t441: db "441: SUB n OVF 2 $"
msg_t442: db "442: SUB n AA-55 $"
msg_t443: db "443: SUB n 55-AA $"
msg_t444: db "444: SUB n 00-80 $"
msg_t445: db "445: SBC n 00 $"
msg_t446: db "446: SBC n 01 $"
msg_t447: db "447: SBC n C=1 $"
msg_t448: db "448: SBC n H=1 $"
msg_t449: db "449: SBC n OVF 1 $"
msg_t450: db "450: SBC n OVF 2 $"
msg_t451: db "451: SBC n 80-80 $"
msg_t452: db "452: SBC n 00-7F $"
msg_t453: db "453: AND n 00 $"
msg_t454: db "454: AND n FF $"
msg_t455: db "455: AND n 55 $"
msg_t456: db "456: AND n AA $"
msg_t457: db "457: AND n 0F $"
msg_t458: db "458: AND n F0 $"
msg_t459: db "459: AND n S=1 $"
msg_t460: db "460: AND n H=1 $"
msg_t461: db "461: XOR n 00 $"
msg_t462: db "462: XOR n FF $"
msg_t463: db "463: XOR n 55 $"
msg_t464: db "464: XOR n AA $"
msg_t465: db "465: XOR n 0F $"
msg_t466: db "466: XOR n F0 $"
msg_t467: db "467: XOR n S=1 $"
msg_t468: db "468: XOR n H=0 $"
msg_t469: db "469: OR n 00 $"
msg_t470: db "470: OR n FF $"
msg_t471: db "471: OR n 55 $"
msg_t472: db "472: OR n AA $"
msg_t473: db "473: OR n 0F $"
msg_t474: db "474: OR n F0 $"
msg_t475: db "475: OR n S=1 $"
msg_t476: db "476: OR n H=0 $"
msg_t477: db "477: CP n 00 $"
msg_t478: db "478: CP n 01 $"
msg_t479: db "479: CP n H=1 $"
msg_t480: db "480: CP n OVF 1 $"
msg_t481: db "481: CP n OVF 2 $"
msg_t482: db "482: CP n AA-55 $"
msg_t483: db "483: CP n 55-AA $"
msg_t484: db "484: CP n 00-80 $"
msg_t485: db "485: ADD HL,BC $"
msg_t486: db "486: ADD HL,DE $"
msg_t487: db "487: ADD HL,HL $"
msg_t488: db "488: ADD HL,SP $"
msg_t489: db "489: ADC HL,BC $"
msg_t490: db "490: ADC HL,DE $"
msg_t491: db "491: ADC HL,HL $"
msg_t492: db "492: ADC HL,SP $"
msg_t493: db "493: SBC HL,BC $"
msg_t494: db "494: SBC HL,DE $"
msg_t495: db "495: SBC HL,HL $"
msg_t496: db "496: SBC HL,SP $"
msg_t497: db "497: ADD HL H=1 $"
msg_t498: db "498: ADD HL C=1 $"
msg_t499: db "499: ADC HL H=1 $"
msg_t500: db "500: ADC HL C=0 $"
msg_t501: db "501: ADC HL C=1 $"
msg_t502: db "502: ADC HL OVF $"
msg_t503: db "503: SBC HL H=1 $"
msg_t504: db "504: SBC HL C=1 $"
msg_t505: db "505: SBC HL OVF $"
msg_t506: db "506: SBC HL Z=1 $"
msg_t507: db "507: ADC HL Z=0 $"
msg_t508: db "508: ADC HL Z=1 $"
msg_t509: db "509: ADD IX,BC $"
msg_t510: db "510: ADD IX,DE $"
msg_t511: db "511: ADD IX,IX $"
msg_t512: db "512: ADD IX,SP $"
msg_t513: db "513: ADD IY,BC $"
msg_t514: db "514: ADD IY,DE $"
msg_t515: db "515: ADD IY,IY $"
msg_t516: db "516: ADD IY,SP $"
msg_t517: db "517: ADD IX C=1 $"
msg_t518: db "518: ADD IY C=1 $"
msg_t519: db "519: ADD IX H=1 $"
msg_t520: db "520: ADD IY H=1 $"
msg_t521: db "521: ADD A,[hl] $"
msg_t522: db "522: ADC A,(IX) $"
msg_t523: db "523: SUB (IY) $"
msg_t524: db "524: CP (IX) $"
msg_t525: db "525: DAA 15+27 $"
msg_t526: db "526: DAA 88+11 $"
msg_t527: db "527: DAA 99+01 $"
msg_t528: db "528: DAA 55+55 $"
msg_t529: db "529: DAA 38+45 $"
msg_t530: db "530: DAA 42-15 $"
msg_t531: db "531: DAA 10-01 $"
msg_t532: db "532: DAA 00-01 $"
msg_t533: db "533: DAA 83-38 $"
msg_t534: db "534: DAA ADC 1 $"
msg_t535: db "535: DAA ADC 2 $"
msg_t536: db "536: DAA SBC 1 $"
msg_t537: db "537: DAA SBC 2 $"
msg_t538: db "538: DAA INC 1 $"
msg_t539: db "539: DAA DEC 1 $"
msg_t540: db "540: DAA INC 2 $"
msg_t541: db "541: CPL 55 $"
msg_t542: db "542: CPL 00 $"
msg_t543: db "543: CPL H=1 $"
msg_t544: db "544: CPL N=1 $"
msg_t545: db "545: NEG 01 $"
msg_t546: db "546: NEG 00 $"
msg_t547: db "547: NEG 80 $"
msg_t548: db "548: NEG C=0 $"
msg_t549: db "549: NEG C=1 $"
msg_t550: db "550: SCF C=1 $"
msg_t551: db "551: SCF+CCF C=0 $"
msg_t552: db "552: CCF C=1 $"
msg_t553: db "553: SCF H=0 $"
msg_t554: db "554: SCF N=0 $"
msg_t555: db "555: CCF H=1 $"
msg_t556: db "556: CCF H=0 $"
msg_t557: db "557: RLA A $"
msg_t558: db "558: RRA A $"
msg_t559: db "559: RLCA A $"
msg_t560: db "560: RRCA A $"
msg_t561: db "561: INC BC 1 $"
msg_t562: db "562: INC BC 2 $"
msg_t563: db "563: INC BC 3 $"
msg_t564: db "564: INC DE 1 $"
msg_t565: db "565: INC DE 2 $"
msg_t566: db "566: INC DE 3 $"
msg_t567: db "567: INC HL 1 $"
msg_t568: db "568: INC HL 2 $"
msg_t569: db "569: INC HL 3 $"
msg_t570: db "570: INC SP $"
msg_t571: db "571: DEC BC 1 $"
msg_t572: db "572: DEC BC 2 $"
msg_t573: db "573: DEC DE 1 $"
msg_t574: db "574: DEC DE 2 $"
msg_t575: db "575: DEC HL 1 $"
msg_t576: db "576: DEC HL 2 $"
msg_t577: db "577: INC IX $"
msg_t578: db "578: INC IY $"
msg_t579: db "579: DEC IX $"
msg_t580: db "580: DEC IY $"
msg_t581: db "581: INC A 1 $"
msg_t582: db "582: INC A Z=1 $"
msg_t583: db "583: INC A V=1 $"
msg_t584: db "584: INC A H=1 $"
msg_t585: db "585: INC B $"
msg_t586: db "586: INC C $"
msg_t587: db "587: INC D $"
msg_t588: db "588: INC E $"
msg_t589: db "589: INC H $"
msg_t590: db "590: INC L $"
msg_t591: db "591: INC [hl] $"
msg_t592: db "592: DEC A 1 $"
msg_t593: db "593: DEC A 2 $"
msg_t594: db "594: DEC A V=1 $"
msg_t595: db "595: DEC A H=1 $"
msg_t596: db "596: DEC B $"
msg_t597: db "597: DEC C $"
msg_t598: db "598: DEC D $"
msg_t599: db "599: DEC E $"
msg_t600: db "600: DEC [hl] $"
msg_t601: db "601: NEG 0xff $"
msg_t602: db "602: NEG 0x00 $"
msg_t603: db "603: NEG 0x55 $"
msg_t604: db "604: BIT 0,0xff $"
msg_t605: db "605: BIT 0,0x00 $"
msg_t606: db "606: BIT 1,0xff $"
msg_t607: db "607: BIT 1,0x00 $"
msg_t608: db "608: BIT 2,0xff $"
msg_t609: db "609: BIT 2,0x00 $"
msg_t610: db "610: BIT 3,0xff $"
msg_t611: db "611: BIT 3,0x00 $"
msg_t612: db "612: BIT 4,0xff $"
msg_t613: db "613: BIT 4,0x00 $"
msg_t614: db "614: BIT 5,0xff $"
msg_t615: db "615: BIT 5,0x00 $"
msg_t616: db "616: BIT 6,0xff $"
msg_t617: db "617: BIT 6,0x00 $"
msg_t618: db "618: BIT 7,0xff $"
msg_t619: db "619: BIT 7,0x00 $"
msg_t620: db "620: RLA (0x00 C1) $"
msg_t621: db "621: RL A (0x00 C1) $"
msg_t622: db "622: RRA (0x00 C1) $"
msg_t623: db "623: RR A (0x00 C1) $"
msg_t624: db "624: XOR 0xAA $"
msg_t625: db "625: XOR 0x55 $"
msg_t626: db "626: SUB 0x80-0xFF $"
msg_t627: db "627: CPI 0xFF (A=0x00) $"
msg_t628: db "628: CP 0x10 (A=0x20) $"
msg_t629: db "629: CALL Z (Z=1) $"
msg_t630: db "630: CALL NZ (Z=0) $"
msg_t631: db "631: CALL M (S=1) $"
msg_t632: db "632: CALL P (S=0) $"
msg_t633: db "633: CALL C (C=1) $"
msg_t634: db "634: CALL NC (C=0) $"
msg_t635: db "635: CALL PE (PV=1) $"
msg_t636: db "636: CALL PO (PV=0) $"
msg_t637: db "637: CP 0x20 (A=0x10) $"
msg_t638: db "638: CP 0x10 (A=0x10) $"
msg_t639: db "639: CP 0x80 (A=0x10) $"
msg_t640: db "640: CP 0x10 (A=0x80) $"
msg_t641: db "641: AND 0x10 (A=0x90) $"
msg_t642: db "642: AND 0x55 (A=0xAA) $"
msg_t643: db "643: AND 0xFF (A=0xFF) $"
msg_t644: db "644: XOR 0x10 (A=0x90) $"
msg_t645: db "645: XOR 0x55 (A=0xAA) $"
msg_t646: db "646: XOR 0xFF (A=0xFF) $"
msg_t647: db "647: OR 0x10 (A=0x90) $"
msg_t648: db "648: OR 0x55 (A=0xAA) $"
msg_t649: db "649: OR 0xFF (A=0xFF) $"
msg_t650: db "650: SUB 0x20 (A=0x10) $"
msg_t651: db "651: SUB 0x10 (A=0x10) $"
msg_t652: db "652: SUB 0x80 (A=0x10) $"
msg_t653: db "653: SUB 0x10 (A=0x80) $"
msg_t654: db "654: SBC 0x20 (C=1 A=0x10) $"
msg_t655: db "655: SBC 0x10 (C=1 A=0x10) $"
msg_t656: db "656: SBC 0x80 (C=1 A=0x10) $"
msg_t657: db "657: SBC 0x10 (C=1 A=0x80) $"
msg_t658: db "658: ADC 0x20 (C=1 A=0x10) $"
msg_t659: db "659: ADC 0x10 (C=1 A=0x10) $"
msg_t660: db "660: ADC 0x80 (C=1 A=0x10) $"
msg_t661: db "661: ADC 0x10 (C=1 A=0x80) $"
msg_t662: db "662: ADC 0x01 (C=1 A=0xFF) $"
msg_t663: db "663: SBC 0x01 (C=1 A=0x00) $"
msg_t664: db "664: ADD 0x01 (A=0x7F) $"
msg_t665: db "665: DAA (A=0x99 + 0x01) $"
msg_t666: db "666: DAA (A=0x50 - 0x25) $"
msg_t667: db "667: DAA (A=0xFF + 0x00) $"
msg_t668: db "668: CPI ([HL]=0x00 BC=1 A=0x00) $"
msg_t669: db "669: CPI ([HL]=0x0,0x1,0x2,0x3 BC=4 A=0x00) $"
msg_t670: db "670: CPIR ([HL]=0x0,0x1,0x2,0x3 BC=4 A=0x00) $"
msg_t671: db "671: CPID ([HL-4]=0x0,0x1,0x2,0x3 BC=4 A=0x00) $"
msg_t672: db "672: CPIR ([HL-1]=0x05,0x10 BC=2 A=0x10) $"

msg_ok:  db "[ OK ]", 0x0d, 0x0a, "$"
msg_err: db "[ FALHA ]", 0x0d, 0x0a, "$"
msg_flags_0: db "[NEW FLAGS: 0x$"
msg_flags_1: db "] $"


section text

; ------------------------------------------------------------
; rotinas de suporte
; ------------------------------------------------------------

reset_flags:
        push hl
        ld l, 0
        ld h, a
        push hl
        pop af
        pop hl
        ret

print:
        ld c, print_c
        call bdos
        ret

is_pass:
        ld de, msg_ok
        ld c, print_c
        call bdos
        ret

is_fail:
        ld de, msg_err
        ld c, print_c
        call bdos
        ld c, 0
        call bdos
        ret

print_flags:
        push bc
        push de
        push hl
        push af
        push de
        push af
        cp d
        jp nz, .fail_1
        ld de, msg_flags_0
        ld c, print_c
        call bdos
        pop bc
        ld a, c
        and 0xd7
        push af
        call print_hex_byte
        ld de, msg_flags_1
        ld c, print_c
        call bdos
        pop af
        pop de
        cp e
        jp nz, .fail_2
        call is_pass
        pop af
        pop hl
        pop de
        pop bc
        ret
        .fail_1:
                call print_hex_byte
                jp is_fail
        .fail_2:
                ld a, e
                call print_hex_byte
                jp is_fail

; --- Sub-rotina: Imprime um byte (A) em Hexa ---
print_hex_byte:
        push af         ; Salva o valor original
        rrca            ; Desloca os 4 bits superiores...
        rrca            ; ...para a posição dos...
        rrca            ; ...4 bits inferiores.
        rrca
        call print_nibble ; Imprime o nibble superior
        pop af          ; Recupera o valor original
        ; Segue para imprimir o nibble inferior

; --- Sub-rotina: Converte e imprime 4 bits (Nibble) ---
print_nibble:
        and 0x0F         ; Máscara para pegar apenas os 4 bits baixos
        cp 10           ; Compara com 10
        jr c, .digit  ; Se < 10, é dígito '0'-'9'
        add a, 7        ; Ajuste para letras 'A'-'F' (diferença ASCII)
        .digit:
        add a, 0x30      ; Adiciona o código ASCII do '0'

        ; Chamada BDOS para imprimir
        ld e, a         ; BDOS espera o caractere em E
        ld c, 2        ; Função 2
        push hl        ; Preserva HL durante a chamada do sistema
        call 5
        pop hl
        ret
