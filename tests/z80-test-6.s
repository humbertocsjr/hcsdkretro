global _main
_main:
    LD D, 0x00          ; D = Old A (Valor inicial do Acumulador: 0 a 255)
LOOP_A:
    LD E, 0x00          ; E = Parâmetro 'n' (O valor a ser comparado: 0 a 255)

LOOP_N:
    ; =========================================================
    ; PASSO 1: Calcular o Resultado Esperado 
    ; H = Expected A, L = Expected F
    ; =========================================================
    
    ; O CP n NUNCA altera o Acumulador. Expected A é sempre igual ao Old A.
    LD H, D             

    ; Calcula as flags matemáticas base (S, Z, H, P/V, C) usando SUB
    LD A, D
    SUB E               
    PUSH AF
    POP BC              ; Guarda as flags geradas pelo SUB no registrador C
    
    ; Aqui corrigimos a "Pegadinha" do Z80 para o CP n
    LD A, C
    AND 0xD7            ; Máscara 1101 0111: Zera os bits 5 e 3 vindos do SUB
    LD L, A             ; Guarda temporariamente em L
    
    LD A, E             ; Pega o nosso parâmetro 'n'
    AND 0x28            ; Máscara 0010 1000: Isola os bits 5 e 3 do parâmetro
    OR L                ; Junta tudo! 
    LD L, A             ; L agora tem o Expected F perfeito para o CP n

    ; =========================================================
    ; PASSO 2: Preparar e Executar a Instrução (SMC)
    ; =========================================================
    ; Como 'n' é um valor imediato, reescrevemos o binário na memória
    PUSH HL
    LD HL, SMC_INSTR + 1
    LD [HL], E          ; Sobrescreve o argumento da instrução CP abaixo
    POP HL
    
    LD A, D             ; Carrega o valor inicial de A

SMC_INSTR:
    CP 0x00             ; >>> INSTRUÇÃO SOB TESTE <<<
                        ; (O 0x00 será alterado em tempo real pelo código acima)

    PUSH AF
    POP BC              ; Agora B = Actual A, C = Actual F

    ; =========================================================
    ; PASSO 3: Validação (Actual vs Expected)
    ; =========================================================
    LD A, B
    CP H                ; Compara A obtido com A esperado
    JP NZ, ERROR_EXIT

    LD A, C
    CP L                ; Compara F obtida com F esperada
    JP NZ, ERROR_EXIT

    ; =========================================================
    ; PASSO 4: Incrementar Loop
    ; =========================================================
    INC E               
    JR NZ, LOOP_N       ; Continua até testar todos os 'n' (0-255)

    INC D               
    JR NZ, LOOP_A       ; Continua até testar todos os 'A' (0-255)

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
    
    ; Imprime " n:" (Parâmetro)
    LD IX, STR_N
    CALL PRINT_STRING
    LD A, E             ; E = Parâmetro 'n' testado
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
STR_OK:   DB "OK! TESTE CP n FINALIZADO COM SUCESSO.", 0
STR_IN:   DB "IN A:", 0
STR_N:    DB " n:", 0       ; Adaptado para mostrar o operando 'n'
STR_F:    DB " F:", 0
STR_EXP:  DB " | EXP A:", 0
STR_FND:  DB " | FND A:", 0