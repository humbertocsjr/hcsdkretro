
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
    SRL A               ; Desloca A para a direita. Bit 7 fica 0.
    LD H, A             ; H guarda o valor temporário
    
    LD A, E             ; Carrega Old F
    AND 0x01            ; Isola a antiga Carry Flag
    JR Z, SKIP_OLD_C    ; Se a antiga Carry era 0, pula a injeção
    LD A, H
    OR 0x80             ; Injeta a antiga Carry no Bit 7
    LD H, A
SKIP_OLD_C:             ; H agora tem o "Expected A" final

    ; --- Expected F ---
    ; 1. Preservar S (bit 7), Z (bit 6) e P/V (bit 2)
    LD A, E
    AND 0xC4            ; Máscara binária 1100 0100
    LD L, A
    
    ; 2. Copiar F3 (bit 3) e F5 (bit 5) do NOVO A
    LD A, H
    AND 0x28            ; Máscara binária 0010 1000
    OR L
    LD L, A
    
    ; 3. A NOVA Flag de Carry (bit 0) recebe o antigo bit 0 do OLD A
    LD A, D
    AND 0x01            ; Isola o bit 0 de Old A
    OR L                ; Junta com o restante das flags
    LD L, A             ; L guarda o "Expected F"

    ; =========================================================
    ; PASSO 2: Executar a instrução no Emulador
    ; =========================================================
    PUSH DE
    POP AF              ; Injeta Old A em A, e Old F em F

    RRA                 ; >>> INSTRUÇÃO SOB TESTE <<<

    PUSH AF
    POP BC              ; Recupera: B = Actual A, C = Actual F

    ; =========================================================
    ; PASSO 3: Validação (Actual vs Expected)
    ; =========================================================
    LD A, B
    CP H                ; Compara A obtido com A esperado
    JP NZ, ERROR_EXIT   ; Se for diferente, ABORTA E IMPRIME ERRO

    LD A, C
    CP L                ; Compara F obtida com F esperada
    JP NZ, ERROR_EXIT   ; Se for diferente, ABORTA E IMPRIME ERRO

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
; ROTINAS DE IMPRESSÃO (Saída via BDOS CP/M - INT 0x05)
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
    JR C, PRINT_NIBBLE_OUT
    ADD A, 7
PRINT_NIBBLE_OUT:
    ; Cai direto no PRINT_CHAR

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
; DADOS ESTÁTICOS
; =========================================================
STR_OK:   DB "OK! TESTE RRA FINALIZADO COM SUCESSO.", 0
STR_IN:   DB "IN A:", 0
STR_F:    DB " F:", 0
STR_EXP:  DB " | EXP A:", 0
STR_FND:  DB " | FND A:", 0