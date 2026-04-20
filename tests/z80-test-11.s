global _main
_main:
    LD D, 0x00          ; D = Valor inicial do Acumulador (Old A)
LOOP_A:
    LD E, 0x00          ; E = Valor inicial das Flags (Old F)
LOOP_F:
    
    ; --- Expected A ---
    LD A, D
    LD [EXP_A_VAR], A   ; O A não é alterado pelo SCF
    
    ; --- Expected F ---
    LD A, E
    AND 0xC4            ; Preserva S, Z e P/V. Zera H, N e C.
    OR 0x01             ; Força a flag de Carry (C=1)
    LD L, A
    
    LD A, D             ; Pega os bits do A atual
    AND 0x28            ; Isola F3 e F5
    OR L
    LD [EXP_F_VAR], A

    ; =========================================================
    ; PASSO 2: Preparar e Executar (O seu emulador assume aqui)
    ; =========================================================
    PUSH DE
    POP AF              ; Injeta D em A e E em F

    SCF                 ; >>> A INSTRUÇÃO SOB TESTE <<<

    ; =========================================================
    ; PASSO 3: Validação
    ; =========================================================
    PUSH AF
    POP HL              ; Agora H = Actual A, L = Actual F
    
    LD A, [EXP_A_VAR]
    CP H
    JP NZ, ERROR_EXIT   ; Se o A divergir, Aborta!

    LD A, [EXP_F_VAR]
    CP L
    JP NZ, ERROR_EXIT   ; Se o F divergir, Aborta!

    ; =========================================================
    ; PASSO 4: Loop de 65.536 iterações
    ; =========================================================
    INC E
    JR NZ, LOOP_F
    INC D
    JR NZ, LOOP_A

    ; SUCESSO TOTAL
    LD IX, STR_OK
    CALL PRINT_STRING
    CALL PRINT_CRLF
    RST 0x00

; =========================================================
; TRATAMENTO DE ERRO (Relatório Completo)
; =========================================================
ERROR_EXIT:
    LD IX, STR_FAIL
    CALL PRINT_STRING
    LD IX, STR_IN_A
    CALL PRINT_STRING
    LD A, D
    CALL PRINT_HEX8
    LD IX, STR_IN_F
    CALL PRINT_STRING
    LD A, E
    CALL PRINT_HEX8
    
    LD IX, STR_EXP_A
    CALL PRINT_STRING
    LD A, [EXP_A_VAR]
    CALL PRINT_HEX8
    LD IX, STR_EXP_F
    CALL PRINT_STRING
    LD A, [EXP_F_VAR]
    CALL PRINT_HEX8
    
    LD IX, STR_FND_A
    CALL PRINT_STRING
    LD A, H
    CALL PRINT_HEX8
    LD IX, STR_FND_F
    CALL PRINT_STRING
    LD A, L
    CALL PRINT_HEX8
    
    CALL PRINT_CRLF
    RST 0x00

; =========================================================
; ROTINAS DE IMPRESSÃO BDOS
; =========================================================
PRINT_STRING:
    LD A, [IX+0]
    OR A
    RET Z
    CALL PRINT_CHAR
    INC IX
    JR PRINT_STRING

PRINT_HEX8:
    PUSH AF
    RRA
    RRA
    RRA
    RRA
    CALL PRINT_NIBBLE
    POP AF
    CALL PRINT_NIBBLE
    RET

PRINT_NIBBLE:
    AND 0x0F
    ADD A, '0'
    CP '9' + 1
    JR C, PNOUT
    ADD A, 7
PNOUT:
    CALL PRINT_CHAR
    RET

PRINT_CHAR:
    PUSH BC
    PUSH DE
    PUSH HL
    LD E, A
    LD C, 0x02
    CALL 0x0005
    POP HL
    POP DE
    POP BC
    RET

PRINT_CRLF:
    LD A, 0x0D
    CALL PRINT_CHAR
    LD A, 0x0A
    CALL PRINT_CHAR
    RET

; =========================================================
; VARIÁVEIS E STRINGS
; =========================================================
EXP_A_VAR:  DB 0x00
EXP_F_VAR:  DB 0x00

STR_OK:     DB "OK! TODAS AS 65536 COMBINACOES PASSARAM.", 0
STR_FAIL:   DB "FALHA!", 0
STR_IN_A:   DB " IN A:", 0
STR_IN_F:   DB " F:", 0
STR_EXP_A:  DB " | EXP A:", 0
STR_EXP_F:  DB " F:", 0
STR_FND_A:  DB " | FND A:", 0
STR_FND_F:  DB " F:", 0