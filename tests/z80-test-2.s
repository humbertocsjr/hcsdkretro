global _main
_main:
    LD D, 0x00          ; D = Old A (0 a 255)
LOOP_A:
    LD E, 0x00          ; E = Old F (0 a 255)

LOOP_F:
    ; =========================================================
    ; PASSO 1: Calcular o Resultado Esperado (Matemática pura)
    ; H = Expected A, L = Expected F
    ; =========================================================
    
    ; --- Expected A ---
    LD A, D
    ADD A, A            ; Shift Left (bit 7 vai para Carry)
    JR NC, SKIP_C_A
    OR 0x01             ; Injeta bit no fim se teve Carry
SKIP_C_A:
    LD H, A             ; H guarda o "Expected A"

    ; --- Expected F ---
    LD A, E
    AND 0xC4            ; Preserva S, Z, P/V
    LD L, A
    
    LD A, H
    AND 0x28            ; F3, F5 baseados no NOVO A
    OR L
    LD L, A
    
    LD A, D
    AND 0x80            ; O bit 7 antigo vira a nova Carry
    JR Z, SKIP_C_F
    LD A, L
    OR 0x01             ; Seta Carry se necessário
    LD L, A
SKIP_C_F:

    ; =========================================================
    ; PASSO 2: Executar a instrução no Emulador
    ; =========================================================
    PUSH DE
    POP AF              ; Injeta D em A, e E em F

    RLCA                ; >>> INSTRUÇÃO SOB TESTE <<<

    PUSH AF
    POP BC              ; Agora B = Actual A, C = Actual F

    ; =========================================================
    ; PASSO 3: Validação (Actual vs Expected)
    ; =========================================================
    LD A, B
    CP H                ; Compara A obtido com A esperado
    JP NZ, ERROR_EXIT   ; Se for diferente, ABORTA

    LD A, C
    CP L                ; Compara F obtido com F esperado
    JP NZ, ERROR_EXIT   ; Se for diferente, ABORTA

    ; =========================================================
    ; PASSO 4: Incrementar Loop
    ; =========================================================
    INC E               
    JR NZ, LOOP_F       

    INC D               
    JR NZ, LOOP_A       

    ; =========================================================
    ; SUCESSO ABSOLUTO
    ; =========================================================
    LD IX, STR_OK
    CALL PRINT_STRING
    CALL PRINT_CRLF
    
    RST 0x00            ; Sai do programa / Volta para o SO


; =========================================================
; TRATAMENTO DE ERRO: Relatório completo e saída
; =========================================================
ERROR_EXIT:
    ; Imprime "IN A:" (Entrada)
    LD IX, STR_IN
    CALL PRINT_STRING
    LD A, D             ; D = Old A
    CALL PRINT_HEX8
    
    ; Imprime " F:"
    LD IX, STR_F
    CALL PRINT_STRING
    LD A, E             ; E = Old F
    CALL PRINT_HEX8
    
    ; Imprime " | EXP A:" (Esperado)
    LD IX, STR_EXP
    CALL PRINT_STRING
    LD A, H             ; H = Expected A
    CALL PRINT_HEX8
    
    ; Imprime " F:"
    LD IX, STR_F
    CALL PRINT_STRING
    LD A, L             ; L = Expected F
    CALL PRINT_HEX8
    
    ; Imprime " | FND A:" (Encontrado)
    LD IX, STR_FND
    CALL PRINT_STRING
    LD A, B             ; B = Actual A
    CALL PRINT_HEX8
    
    ; Imprime " F:"
    LD IX, STR_F
    CALL PRINT_STRING
    LD A, C             ; C = Actual F
    CALL PRINT_HEX8
    
    CALL PRINT_CRLF
    RST 0x00            ; ABORTA O PROGRAMA APÓS O ERRO


; =========================================================
; ROTINAS DE IMPRESSÃO (Saída via BDOS)
; =========================================================

; Imprime uma string terminada em zero apontada por IX
PRINT_STRING:
    LD A, [IX+0]        ; Carrega caractere da string
    OR A                ; Verifica se é 0x00 (Null terminator)
    RET Z               ; Se for zero, finaliza a rotina
    CALL PRINT_CHAR     ; Imprime caractere
    INC IX              ; Avança ponteiro
    JR PRINT_STRING     ; Repete para o próximo caractere

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
    JR C, PRINT_NIBBLE_OUT
    ADD A, 7
PRINT_NIBBLE_OUT:
    ; Cai direto na rotina de PRINT_CHAR e usa o RET dela para voltar

PRINT_CHAR:
    PUSH BC
    PUSH DE
    PUSH HL
    LD E, A             ; BDOS Função 2 espera o char em E
    LD C, 0x02          
    CALL 0x0005         ; Chamada de sistema CP/M (BDOS)
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
; DADOS (Strings estáticas terminadas em 0x00)
; =========================================================
STR_OK:   DB "OK! TODAS AS COMBINACOES PASSARAM.", 0
STR_IN:   DB "IN A:", 0
STR_F:    DB " F:", 0
STR_EXP:  DB " | EXP A:", 0
STR_FND:  DB " | FND A:", 0