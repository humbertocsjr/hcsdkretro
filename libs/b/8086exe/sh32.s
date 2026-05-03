; 32-bit shift left
; Stack at entry:
;   [SP+0] = ret_addr
;   [SP+2] = primary_low   (= b = right operand = shift count)
;   [SP+4] = primary_high
;   [SP+6] = secondary_low (= a = left operand = value to shift)
;   [SP+8] = secondary_high
; After push bp: [bp+4]=primary_low [bp+6]=primary_high
;                [bp+8]=secondary_low [bp+10]=secondary_high
; Computes: DX:AX = secondary << primary = a << b
global __shl32

section text
__shl32:
    push bp
    mov bp, sp
    push cx

    ; Load shift count (primary_low = [bp+4])
    mov cx, [bp+4]
    and cx, 31
    jcxz .shl_done

    ; Load value (secondary = [bp+8]:[bp+10])
    mov ax, [bp+8]     ; low
    mov dx, [bp+10]    ; high

.shl_loop:
    shl ax, 1
    rcl dx, 1
    loop .shl_loop

.shl_done:
    pop cx
    pop bp
    ret

; 32-bit shift right (unsigned)
; Computes: DX:AX = secondary >> primary = a >> b
global __shr32

__shr32:
    push bp
    mov bp, sp
    push cx

    ; Load shift count (primary_low = [bp+4])
    mov cx, [bp+4]
    and cx, 31
    jcxz .shr_done

    ; Load value (secondary = [bp+8]:[bp+10])
    mov ax, [bp+8]
    mov dx, [bp+10]

.shr_loop:
    shr dx, 1
    rcr ax, 1
    loop .shr_loop

.shr_done:
    pop cx
    pop bp
    ret
