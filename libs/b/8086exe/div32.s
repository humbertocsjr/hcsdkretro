; 32-bit unsigned divide and modulo
; Stack after call + push bp:
;   [bp+4]  = primary_low   (= b = right operand = divisor)
;   [bp+6]  = primary_high
;   [bp+8]  = secondary_low (= a = left operand = dividend)
;   [bp+10] = secondary_high
; Computes: DX:AX = secondary / primary = a / b
; Uses repeated subtraction (simple, reliable)
global __udiv32
global __umod32

section text
__udiv32:
    push bp
    mov bp, sp
    push si
    push di

    ; divisor = primary = [bp+4]:[bp+6]
    mov bx, [bp+4]     ; divisor_low
    mov cx, [bp+6]     ; divisor_high

    ; Check division by zero
    mov ax, bx
    or ax, cx
    jnz .div_ok
    mov ax, 0xFFFF
    mov dx, 0xFFFF
    jmp .div_exit

.div_ok:
    ; dividend = secondary = [bp+8]:[bp+10]
    mov si, [bp+8]     ; dividend_low
    mov di, [bp+10]    ; dividend_high

    ; quotient = 0
    xor ax, ax
    xor dx, dx

.div_loop:
    ; if dividend < divisor: done
    cmp di, cx
    jb .div_done
    ja .div_sub
    cmp si, bx
    jb .div_done

.div_sub:
    ; dividend -= divisor
    sub si, bx
    sbb di, cx
    ; quotient++
    add ax, 1
    adc dx, 0
    jmp .div_loop

.div_done:
    ; Remainder is in DI:SI (for mod32)

.div_exit:
    pop di
    pop si
    pop bp
    ret

; 32-bit unsigned modulo
__umod32:
    push bp
    mov bp, sp
    push si
    push di

    mov bx, [bp+4]     ; divisor_low
    mov cx, [bp+6]     ; divisor_high

    mov ax, bx
    or ax, cx
    jnz .mod_ok
    xor ax, ax
    xor dx, dx
    jmp .mod_exit

.mod_ok:
    mov si, [bp+8]     ; dividend_low
    mov di, [bp+10]    ; dividend_high

.mod_loop:
    cmp di, cx
    jb .mod_done
    ja .mod_sub
    cmp si, bx
    jb .mod_done

.mod_sub:
    sub si, bx
    sbb di, cx
    jmp .mod_loop

.mod_done:
    mov ax, si
    mov dx, di

.mod_exit:
    pop di
    pop si
    pop bp
    ret
