; 32-bit unsigned multiply
; Input:  primary on stack (DX:AX pushed: low at SP+0, high at SP+2)
;         secondary on stack (CX:BX pushed: low at SP+4, high at SP+6)
; Output: DX:AX = result
; Clobbers: CX, BX
global __mul32u

section text
__mul32u:
    push bp
    mov bp, sp
    push si

    ; Load primary: AX = low, DX = high (already in regs from caller?)
    ; Actually, caller pushes DX:AX and CX:BX before call.
    ; Stack: ret_addr(2) + bp(2) + si(2) + primary_low + primary_high + sec_low + sec_high
    ;   [bp+4] = primary_low, [bp+6] = primary_high
    ;   [bp+8] = secondary_low, [bp+10] = secondary_high

    ; p = primary (a = high:low = [bp+6]:[bp+4])
    ; s = secondary (b = high:low = [bp+10]:[bp+8])

    ; result_low = p_low * s_low
    mov ax, [bp+4]      ; p_low
    mov bx, [bp+8]      ; s_low
    mul bx              ; DX:AX = p_low * s_low
    mov si, dx          ; SI = partial result_high (temp_high)
    ; AX = result_low (final)

    ; result_high += p_low * s_high
    mov cx, ax          ; save result_low
    mov ax, [bp+4]      ; p_low
    mov bx, [bp+10]     ; s_high
    mul bx              ; DX:AX = p_low * s_high
    add si, ax          ; add to result_high

    ; result_high += p_high * s_low
    mov ax, [bp+6]      ; p_high
    mov bx, [bp+8]      ; s_low
    mul bx              ; DX:AX = p_high * s_low
    add si, ax          ; add to result_high

    ; DX = result_high, AX = result_low
    mov ax, cx          ; restore result_low
    mov dx, si          ; result_high

    pop si
    pop bp
    ret
