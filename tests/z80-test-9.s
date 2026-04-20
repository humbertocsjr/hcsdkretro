global _main
_main:
    ; =========================================================
    ; TESTE 1: Exaustão Pura (Caminhando para trás, BC = 0)
    ; =========================================================
    LD A, 1
    LD [TEST_ID], A
    LD HL, DATA_MEM
    LD [HL], 0x02      ; Byte no endereço 0x2000
    INC HL
    LD [HL], 0x01      ; Byte no endereço 0x2001 (Final)
    
    LD HL, DATA_MEM + 1 ; HL aponta para o FINAL do bloco
    LD A, 0x05          ; Procura 0x05 (não existe)
    LD BC, 0x0002       
    OR A                ; C = 0
    
    CPDR                ; Executa busca para trás
    
    CALL SAVE_ACTUAL
    LD HL, 0x0000       ; Esperado: BC = 0
    LD [EXP_BC], HL
    LD HL, DATA_MEM - 1 ; Esperado: HL decrementou 2 vezes (2001 -> 2000 -> 1FFF)
    LD [EXP_HL], HL
    LD A, 0x22          ; Flags: Z=0, P/V=0, N=1, F5=1
    LD [EXP_F], A
    CALL VALIDATE

    ; =========================================================
    ; TESTE 2: Match no Meio do Caminho (P/V=1)
    ; =========================================================
    LD A, 2
    LD [TEST_ID], A
    LD HL, DATA_MEM + 2
    LD [HL], 0x05       ; Coloca o alvo no endereço 2002
    
    LD HL, DATA_MEM + 5 ; Começa no 2005
    LD A, 0x05          
    LD BC, 0x0006       
    SCF                 ; C = 1
    
    CPDR
    
    CALL SAVE_ACTUAL
    LD HL, 0x0002       ; Esperado: BC = 2 (Parou após 4 iterações: 5,4,3,2)
    LD [EXP_BC], HL
    LD HL, DATA_MEM + 1 ; HL parou no 2001 (decrementou após o match no 2002)
    LD [EXP_HL], HL
    LD A, 0x47          ; Flags: Z=1, P/V=1, N=1, C=1
    LD [EXP_F], A
    CALL VALIDATE

    ; =========================================================
    ; TESTE 3: Match no limite de BC
    ; =========================================================
    LD A, 3
    LD [TEST_ID], A
    LD HL, DATA_MEM
    LD [HL], 0xAA
    
    LD HL, DATA_MEM
    LD A, 0xAA
    LD BC, 0x0001
    OR A
    
    CPDR
    
    CALL SAVE_ACTUAL
    LD HL, 0x0000
    LD [EXP_BC], HL
    LD HL, DATA_MEM - 1
    LD [EXP_HL], HL
    LD A, 0x42          ; Z=1, P/V=0
    LD [EXP_F], A
    CALL VALIDATE

    ; SUCESSO TOTAL
    LD IX, STR_OK
    CALL PRINT_STRING
    CALL PRINT_CRLF
    RST 0x00


; =========================================================
; ROTINAS DE SUPORTE
; =========================================================
SAVE_ACTUAL:
    LD [ACT_BC], BC
    LD [ACT_HL], HL
    PUSH AF
    POP DE
    LD A, E
    LD [ACT_F], A
    RET

VALIDATE:
    LD A, [EXP_F]
    LD HL, ACT_F
    CP [HL]
    JP NZ, ERROR_EXIT
    LD HL, [EXP_BC]
    LD DE, [ACT_BC]
    OR A
    SBC HL, DE
    JP NZ, ERROR_EXIT
    LD HL, [EXP_HL]
    LD DE, [ACT_HL]
    OR A
    SBC HL, DE
    JP NZ, ERROR_EXIT
    RET

ERROR_EXIT:
    LD IX, STR_FAIL
    CALL PRINT_STRING
    LD A, [TEST_ID]
    CALL PRINT_HEX8
    LD IX, STR_EXP_BC
    CALL PRINT_STRING
    LD HL, [EXP_BC]
    CALL PRINT_HEX16
    LD IX, STR_HL
    CALL PRINT_STRING
    LD HL, [EXP_HL]
    CALL PRINT_HEX16
    LD IX, STR_F
    CALL PRINT_STRING
    LD A, [EXP_F]
    CALL PRINT_HEX8
    LD IX, STR_FND_BC
    CALL PRINT_STRING
    LD HL, [ACT_BC]
    CALL PRINT_HEX16
    LD IX, STR_HL
    CALL PRINT_STRING
    LD HL, [ACT_HL]
    CALL PRINT_HEX16
    LD IX, STR_F
    CALL PRINT_STRING
    LD A, [ACT_F]
    CALL PRINT_HEX8
    CALL PRINT_CRLF
    RST 0x00

PRINT_STRING:
    LD A, [IX+0]
    OR A
    RET Z
    CALL PRINT_CHAR
    INC IX
    JR PRINT_STRING

PRINT_HEX16:
    LD A, H
    CALL PRINT_HEX8
    LD A, L
    CALL PRINT_HEX8
    RET

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

TEST_ID: DB 0
EXP_BC:  DW 0
EXP_HL:  DW 0
EXP_F:   DB 0
ACT_BC:  DW 0
ACT_HL:  DW 0
ACT_F:   DB 0
STR_OK:     DB "OK! CPDR ESTA PERFEITO.", 0
STR_FAIL:   DB "FAIL TEST:", 0
STR_EXP_BC: DB " | EXP BC:", 0
STR_FND_BC: DB " | FND BC:", 0
STR_HL:     DB " HL:", 0
STR_F:      DB " F:", 0
DATA_MEM: EQU 0x2000