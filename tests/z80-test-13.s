global _main
_main:
    LD IX, DAA_TESTS
    XOR A
    LD [TEST_IDX], A    ; Zera o contador de testes

TEST_LOOP:
    ; Verifica se já rodamos todos os 7 testes
    LD A, [TEST_IDX]
    CP 0x07             ; Temos 7 edge-cases cravados
    JR Z, TESTS_DONE
    
    ; Carrega a configuração do teste atual
    LD D, [IX+0]        ; Input A
    LD E, [IX+1]        ; Input F
    LD B, [IX+2]        ; Expected A
    LD C, [IX+3]        ; Expected F
    
    ; Prepara o hardware simulado
    PUSH DE
    POP AF              ; A = In A, F = In F
    
    DAA                 ; >>> A INSTRUÇÃO SOB TESTE <<<
    
    PUSH AF
    POP HL              ; H = Actual A, L = Actual F
    
    ; Validação do Acumulador
    LD A, B
    CP H
    JP NZ, ERROR_EXIT
    
    ; Validação das Flags
    LD A, C
    CP L
    JP NZ, ERROR_EXIT
    
    ; Avança para o próximo teste
    LD A, [TEST_IDX]
    INC A
    LD [TEST_IDX], A    ; Incrementa contador
    
    INC IX
    INC IX
    INC IX
    INC IX ; Avança 4 bytes na tabela
    JR TEST_LOOP

TESTS_DONE:
    LD IX, STR_OK
    CALL PRINT_STRING
    CALL PRINT_CRLF
    RST 0x00            ; Fim com sucesso

; =========================================================
; TRATAMENTO DE ERRO (Relatório Estendido)
; =========================================================
ERROR_EXIT:
    LD IX, STR_FAIL
    CALL PRINT_STRING
    
    ; Imprime qual teste falhou (1 a 7)
    LD A, [TEST_IDX]
    INC A               ; Para mostrar de 1 a 7 na tela
    CALL PRINT_HEX8
    
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
    LD A, B
    CALL PRINT_HEX8
    LD IX, STR_EXP_F
    CALL PRINT_STRING
    LD A, C
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
    RST 0x00            ; Aborta a execução para não entrar em loop!

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
; TABELA DOS 7 EDGE CASES MORTAIS DO DAA
; Formato: IN_A, IN_F, EXP_A, EXP_F
; =========================================================
DAA_TESTS:
    ; 1: Adição BCD Válida (N=0). Nenhuma correção.
    DB 0x15, 0x00, 0x15, 0x00 
    
    ; 2: Estouro do Nibble Inferior (A=0x1A). Adiciona 0x06. Gera Half-Carry.
    DB 0x1A, 0x00, 0x20, 0x30 

    ; 3: Estouro do Nibble Superior (A=0xA1). Adiciona 0x60. Gera Carry total.
    DB 0xA1, 0x00, 0x01, 0x01 

    ; 4: Duplo Estouro BCD (A=0x9B). Adiciona 0x66. Gera Half-Carry e Carry.
    DB 0x9B, 0x00, 0x01, 0x11 

    ; 5: Subtração Válida (N=1). Nenhuma correção necessária.
    DB 0x15, 0x02, 0x15, 0x02 

    ; 6: Subtração com Half-Carry prévio (N=1, H=1). Subtrai 0x06. O H deve zerar após!
    DB 0x1F, 0x12, 0x19, 0x0A 

    ; 7: Subtração com Carry prévio (N=1, C=1). Subtrai 0x60. Carry é preservado, S=1.
    DB 0xF5, 0x03, 0x95, 0x87 

; =========================================================
; VARIÁVEIS E STRINGS
; =========================================================
TEST_IDX:   DB 0x00
STR_OK:     DB "OK! DAA ESTA PERFEITO.", 0
STR_FAIL:   DB "FALHA TESTE ", 0
STR_IN_A:   DB " | IN A:", 0
STR_IN_F:   DB " F:", 0
STR_EXP_A:  DB " | EXP A:", 0
STR_EXP_F:  DB " F:", 0
STR_FND_A:  DB " | FND A:", 0
STR_FND_F:  DB " F:", 0