; Simple calculator for CP/M 2.2
; Reads two numbers and an operator (+, -, *, /)
; Displays the result.
; Numbers are 16-bit signed integers.
; Uses BDOS calls for I/O.
; Assembler syntax: [] for indirect addressing.

BDOS    equ     5
C_READ  equ     10          ; Read console buffer
C_WRITE equ     9           ; Print string
C_CONOUT equ    2           ; Console output character


Start:
        ld      [SaveSP], sp        ; Save stack pointer for error recovery

        ; Prompt for first number
        ld      de, MsgNum1
        ld      c, C_WRITE
        call    BDOS
        call    ReadLine
        call    ParseNumber
        ld      [Num1], hl          ; Store first number

        ; Prompt for second number
        ld      de, MsgNum2
        ld      c, C_WRITE
        call    BDOS
        call    ReadLine
        call    ParseNumber
        ld      [Num2], hl          ; Store second number

        ; Prompt for operator
        ld      de, MsgOp
        ld      c, C_WRITE
        call    BDOS
        call    ReadLine
        ld      a, [Buffer+2]       ; First character of input
        ld      [Operator], a

        ; Perform the operation
        ld      hl, [Num1]
        ld      de, [Num2]
        ld      a, [Operator]
        cp      '+'
        jr      z, DoAdd
        cp      '-'
        jr      z, DoSub
        cp      '*'
        jr      z, DoMul
        cp      '/'
        jr      z, DoDiv
        ; Invalid operator
        ld      de, MsgInvalidOp
        ld      c, C_WRITE
        call    BDOS
        jp      Start

DoAdd:
        add     hl, de
        jr      PrintResult
DoSub:
        or      a               ; clear carry
        sbc     hl, de
        jr      PrintResult
DoMul:
        call    Multiply        ; HL = HL * DE
        jr      PrintResult
DoDiv:
        ld      a, d
        or      e
        jr      z, DivZero      ; Division by zero
        call    Divide          ; HL = HL / DE
        jr      PrintResult

DivZero:
        ld      de, MsgDivZero
        ld      c, C_WRITE
        call    BDOS
        jp      Start

PrintResult:
        push    hl              ; Save result
        ld      de, MsgResult
        ld      c, C_WRITE
        call    BDOS
        pop     hl
        call    PrintHL         ; Print HL as signed decimal
        ld      a, 13
        call    PutChar
        ld      a, 10
        call    PutChar
        jp      Start           ; Loop for next calculation

; ------------------------------------------------------------
; Read a line from console into Buffer
; ------------------------------------------------------------
ReadLine:
        ld      de, Buffer
        ld      c, C_READ
        call    BDOS
        ; Ensure null terminator at end
        ld      a, [Buffer+1]   ; Number of characters read
        ld      hl, Buffer+2
        ld      e, a
        ld      d, 0
        add     hl, de
        ld      [hl], 0         ; Terminate with null
        ret

; ------------------------------------------------------------
; Parse a signed decimal number from Buffer+2
; Returns HL = value
; ------------------------------------------------------------
ParseNumber:
        ld      hl, 0
        ld      de, Buffer+2
        ld      a, [de]         ; First character
        cp      '-'
        jr      nz, .positive
        inc     de              ; Skip minus
        call    .parseUnsigned
        ; Negate HL
        ld      a, l
        cpl
        ld      l, a
        ld      a, h
        cpl
        ld      h, a
        inc     hl
        ret
.positive:
        call    .parseUnsigned
        ret

.parseUnsigned:
        ; Parse unsigned number from [DE] until non-digit
        ld      hl, 0
.loop:
        ld      a, [de]
        cp      '0'
        jr      c, .done
        cp      '9'+1
        jr      nc, .done
        sub     '0'
        ld      c, a
        push    bc
        ; HL = HL * 10
        add     hl, hl          ; *2
        ld      b, h
        ld      c, l
        add     hl, hl          ; *4
        add     hl, hl          ; *8
        add     hl, bc          ; *10
        pop     bc
        ld      b, 0
        add     hl, bc          ; + digit
        inc     de
        jr      .loop
.done:
        ret

; ------------------------------------------------------------
; Multiply signed 16-bit: HL = HL * DE
; Uses absolute values and adjusts sign.
; ------------------------------------------------------------
Multiply:
        push    hl
        push    de
        ; Sign of HL
        pop     hl
        ld      a, h
        or      a
        jp      p, .hl_pos
        ; Negate HL
        ld      a, l
        cpl
        ld      l, a
        ld      a, h
        cpl
        ld      h, a
        inc     hl
        ld      a, 1
        jr      .store_hl_sig
.hl_pos:
        xor     a
.store_hl_sig:
        ld      b, a            ; B = sign of HL (0=pos, 1=neg)
        ; Sign of DE
        pop     de
        ld      a, d
        or      a
        jp      p, .de_pos
        ld      a, e
        cpl
        ld      e, a
        ld      a, d
        cpl
        ld      d, a
        inc     de
        ld      a, 1
        jr      .store_de_sig
.de_pos:
        xor     a
.store_de_sig:
        ld      c, a            ; C = sign of DE
        push    bc              ; Save signs
        ; Now HL and DE are absolute values
        ; Multiply unsigned 16x16, result in HL (low 16 bits)
        ld      b, h
        ld      c, l            ; BC = multiplier
        ld      hl, 0           ; Product
        ld      a, 16
.mul_loop:
        srl     b
        rr      c
        jr      nc, .no_add
        add     hl, de
.no_add:
        sla     e
        rl      d
        dec     a
        jr      nz, .mul_loop
        pop     bc              ; Restore signs
        ; Apply sign: result negative if B XOR C = 1
        ld      a, b
        xor     c
        or      a
        jr      z, .done
        ; Negate HL
        ld      a, l
        cpl
        ld      l, a
        ld      a, h
        cpl
        ld      h, a
        inc     hl
.done:
        ret

; ------------------------------------------------------------
; Divide signed 16-bit: HL = HL / DE (quotient)
; Uses absolute values and repeated subtraction.
; ------------------------------------------------------------
Divide:
        ; Check divisor zero (already done in caller)
        push    hl
        push    de
        ; Sign of dividend (HL)
        pop     hl
        ld      a, h
        or      a
        jp      p, .divid_pos
        ld      a, l
        cpl
        ld      l, a
        ld      a, h
        cpl
        ld      h, a
        inc     hl
        ld      a, 1
        jr      .store_divid_sig
.divid_pos:
        xor     a
.store_divid_sig:
        ld      b, a            ; B = sign of dividend
        ; Sign of divisor (DE)
        pop     de
        ld      a, d
        or      a
        jp      p, .divis_pos
        ld      a, e
        cpl
        ld      e, a
        ld      a, d
        cpl
        ld      d, a
        inc     de
        ld      a, 1
        jr      .store_divis_sig
.divis_pos:
        xor     a
.store_divis_sig:
        ld      c, a            ; C = sign of divisor
        push    bc              ; save signs
        ; Now HL = dividend (unsigned), DE = divisor (unsigned)
        ; Repeated subtraction
        ld      bc, 0           ; quotient
.sub_loop:
        ; Compare HL with DE
        ld      a, h
        cp      d
        jr      c, .sub_done
        jr      nz, .can_sub
        ld      a, l
        cp      e
        jr      c, .sub_done
.can_sub:
        ; HL >= DE, subtract
        or      a
        sbc     hl, de
        inc     bc
        jr      .sub_loop
.sub_done:
        ; Quotient in BC, remainder in HL (ignored)
        ; Apply sign
        pop     af              ; A = sign byte (B in lower, C in upper? Actually we pushed BC, so pop into BC)
        ; We need to restore the signs properly. We pushed BC, so pop into BC.
        pop     bc
        ld      a, b
        xor     c
        or      a
        jr      z, .positive
        ; Negate quotient
        ld      a, c
        cpl
        ld      c, a
        ld      a, b
        cpl
        ld      b, a
        inc     bc
.positive:
        push    bc
        pop     hl              ; HL = quotient
        ret

; ------------------------------------------------------------
; Print HL as signed decimal number
; Uses a separate division by 10 routine (not the general Divide)
; ------------------------------------------------------------
PrintHL:
        ld      a, h
        or      a
        jp      p, .positive
        push    hl
        ld      a, '-'
        call    PutChar
        pop     hl
        ; Negate HL
        ld      a, l
        cpl
        ld      l, a
        ld      a, h
        cpl
        ld      h, a
        inc     hl
.positive:
        ; Convert to decimal digits on stack
        ld      bc, 0           ; C = digit count
.convert:
        call    Div10           ; HL = HL/10, A = remainder
        add     a, '0'
        push    af
        inc     c
        ld      a, h
        or      l
        jr      nz, .convert
.print:
        pop     af
        call    PutChar
        dec     c
        jr      nz, .print
        ret

; ------------------------------------------------------------
; Divide HL by 10, return quotient in HL, remainder in A
; Uses repeated subtraction (fast enough for 16-bit)
; ------------------------------------------------------------
Div10:
        ld      de, 10
        ld      bc, 0
.loop:
        ld      a, h
        cp      d
        jr      c, .try_lower
        jr      nz, .sub
        ld      a, l
        cp      e
        jr      c, .try_lower
.sub:
        or      a
        sbc     hl, de
        inc     bc
        jr      .loop
.try_lower:
        ; Here HL < 10
        push    hl              ; remainder
        push    bc
        pop     hl              ; quotient
        pop     af              ; remainder in A (low byte of HL)
        ret

; ------------------------------------------------------------
; Output a character in A
; ------------------------------------------------------------
PutChar:
        push    hl
        push    de
        ld      e, a
        ld      c, C_CONOUT
        call    BDOS
        pop     de
        pop     hl
        ret

section data

; ------------------------------------------------------------
; Data area
; ------------------------------------------------------------
MsgNum1:        db      "Enter first number: $"
MsgNum2:        db      "Enter second number: $"
MsgOp:          db      "Enter operator (+, -, *, /): $"
MsgResult:      db      "Result: $"
MsgInvalidOp:   db      "Invalid operator!$",13,10,"$"
MsgDivZero:     db      "Division by zero!$",13,10,"$"
Buffer:         db      80, 0          ; Max length, actual length
                ds      80              ; Buffer space
SaveSP:         dw      0
Num1:           dw      0
Num2:           dw      0
Operator:       db      0
