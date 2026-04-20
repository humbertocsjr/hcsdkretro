global _main
_main:
    LD D, 0x00          ; D = Valor do Acumulador (A)
LOOP_A:
    LD E, 0x00          ; E = Valor da Memória [HL]
LOOP_HL:

    ; =========================================================
    ; PASSO 1: O Gabarito Perfeito (Matemática Pura)
    ; Calcula as flags exatas exigidas pelo silício para A e [HL]
    ; =========================================================
    
    ; 1. Extrai as flags normais (S, Z, H, N) usando a instrução CP base
    LD A, D
    CP E
    PUSH AF
    POP HL              ; L agora contém as flags base geradas pelo CP
    LD A, L
    AND 0xD2            ; Máscara 1101 0010 (Preserva S, Z, H e força N=1)
    LD C, A             ; C = Nosso "Expected F" em construção temporária

    ; 2. O cálculo secreto das flags F3/F5 (TMP = A - [HL] - H_Flag)
    LD A, D
    SUB E               ; A = A - [HL]
    BIT 4, L            ; Testa se ocorreu Half-Carry (Bit 4) no CP original
    JR Z, NO_HALF_CARRY
    DEC A               ; Se ocorreu, subtrai o H_Flag (TMP = A - [HL] - 1)
NO_HALF_CARRY:
    LD B, A             ; B guarda o valor final de TMP

    ; 3. Extrai F3 (Bit 3 de TMP)
    AND 0x08            ; Isola o Bit 3
    OR C
    LD C, A             ; Injeta no Expected F temporário

    ; 4. Extrai F5 (Bit 1 de TMP movido para a posição do Bit 5)
    LD A, B
    AND 0x02            ; Isola o Bit 1
    RLCA
    RLCA
    RLCA
    RLCA  ; Joga o Bit 1 para a posição do Bit 5
    OR C
    LD C, A             ; Injeta no Expected F temporário

    ; 5. Força P/V=1 e C=0 (Porque nosso teste usará BC=2 e C=0 inicial)
    OR 0x04             ; Seta a Flag P/V (Bit 2)
    LD [EXP_F_VAR], A   ; SALVA O GABARITO FINAL NA RAM! Blindado contra a CPU.

    ; =========================================================
    ; PASSO 2: Preparar o Emulador para o Teste
    ; =========================================================
    LD HL, DATA_MEM
    LD [HL], E          ; Coloca o valor do loop atual na memória
    
    XOR A               ; Limpa a Carry Flag e reseta o registrador F
    LD BC, 0x0002       ; Inicia BC em 2 (Assim a CPU faz BC=1 internamente, logo P/V=1)
    LD A, D             ; Injeta o valor do loop (D) no Acumulador
    
    CPI                 ; >>> A INSTRUÇÃO FINAL SOB TESTE <<<

    ; =========================================================
    ; PASSO 3: Validação (Encontrado vs Esperado)
    ; =========================================================
    PUSH AF
    POP HL              ; L recebe o FND_F (Actual Flags geradas pelo emulador)
    
    LD A, [EXP_F_VAR]   ; Carrega o gabarito blindado da memória
    CP L                ; Compara com Actual F
    JP NZ, ERROR_EXIT   ; SE UM ÚNICO BIT DIVERGIR, ABORTA IMEDIATAMENTE!

    ; =========================================================
    ; PASSO 4: Loop de 65.536 iterações
    ; =========================================================
    INC E
    JR NZ, LOOP_HL      ; Volta e testa o próximo valor de memória
    
    INC D
    JR NZ, LOOP_A       ; Volta e testa o próximo valor no Acumulador

    ; =========================================================
    ; SUCESSO TOTAL
    ; =========================================================
    LD IX, STR_OK
    CALL PRINT_STRING
    CALL PRINT_CRLF
    RST 0x00

; =========================================================
; TRATAMENTO DE ERRO (Relatório de Autópsia)
; =========================================================
ERROR_EXIT:
    LD IX, STR_FAIL
    CALL PRINT_STRING
    
    LD IX, STR_A
    CALL PRINT_STRING
    LD A, D             ; Input A que causou a falha
    CALL PRINT_HEX8
    
    LD IX, STR_HL_VAL
    CALL PRINT_STRING
    LD A, E             ; Input [HL] que causou a falha
    CALL PRINT_HEX8
    
    LD IX, STR_EXP
    CALL PRINT_STRING
    LD A, [EXP_F_VAR]   ; A Flag matemática correta
    CALL PRINT_HEX8
    
    LD IX, STR_FND
    CALL PRINT_STRING
    LD A, L             ; A Flag falha que seu emulador gerou
    CALL PRINT_HEX8
    
    CALL PRINT_CRLF
    RST 0x00

; =========================================================
; ROTINAS DE IMPRESSÃO BDOS (Console via INT 0x05)
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
; VARIÁVEIS NA MEMÓRIA E STRINGS DE TEXTO
; =========================================================
EXP_F_VAR:  DB 0x00
DATA_MEM:   DB 0x00

STR_OK:     DB "OK! TODAS AS 65536 COMBINACOES DO CPI PASSARAM.", 0
STR_FAIL:   DB "FALHA CRITICA MATEMATICA!", 0
STR_A:      DB " | A:", 0
STR_HL_VAL: DB " [HL]:", 0
STR_EXP:    DB " | EXP F:", 0
STR_FND:    DB " | FND F:", 0