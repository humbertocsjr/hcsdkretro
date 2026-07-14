;; Teste para JNC (Jump if Not Carry) e JC (Jump if Carry)
section text
global _start
_start:
    clc                ; Clear carry flag (CF=0)
    jnc  no_carry      ; Should jump if CF=0
    mov  ax, 0xFFFF    ; Error: shouldn't reach here
no_carry:
    stc                ; Set carry flag (CF=1)
    jc   has_carry     ; Should jump if CF=1
    mov  ax, 0xFFFF    ; Error: shouldn't reach here
has_carry:
    mov  ax, 0x1234    ; Success
    ret
