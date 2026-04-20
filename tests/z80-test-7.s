global _main
_main:
    ; Zera as variáveis de controle
    XOR A
    LD [C_FLAG], A      ; Testa com Carry = 0 e Carry = 1
LOOP_C:
    LD A, 0x01
    LD [BC_VAL], A      ; Testa com BC = 1 (P/V zera) e BC = 2 (P/V=1)
LOOP_BC:
    LD D, 0x00          ; D = Old A (0 a 255)
LOOP_A:
    LD E, 0x00          ; E = Valor da Memória [HL] (0 a 255)
LOOP_MEM:

    ; =========================================================
    ; PASSO 1: O Gabarito Matemático (As regras ocultas do CPI)
    ; =========================================================
    CALL CALC_EXPECTED_FLAGS

    ; =========================================================
    ; PASSO 2: Preparar o Estado do Emulador
    ; =========================================================
    ; 2.1 Configura a Memória
    LD HL, 0x2000
    LD [HL], E          ; Coloca o valor de teste na memória

    ; 2.2 Configura o Contador (BC)
    LD A, [BC_VAL]
    LD C, A
    LD B, 0x00          ; BC pronto!
    
    ; 2.3 Configura o Acumulador e a Carry Flag Antiga (CORRIGIDO)
    LD H, D             ; H vai para A (Então H recebe o Old A)
    
    LD A, [C_FLAG]
    LD L, A             ; L vai para F (Então L recebe o C_FLAG)
    
    PUSH HL
    POP AF              ; Agora sim: A = Old A, F = C_FLAG!
    ; 2.4 Garante que HL aponta para os dados
    LD HL, 0x2000

    ; =========================================================
    ; PASSO 3: Executar!
    ; =========================================================
    CPI                 ; >>> INSTRUÇÃO SOB TESTE <<<

    ; =========================================================
    ; PASSO 4: Validação (Actual vs Expected)
    ; =========================================================
    PUSH AF
    POP HL              ; Agora H = Actual A, L = Actual F
    
    ; Valida as Flags (Onde 99% dos emuladores falham)
    LD A, [EXP_F]
    CP L
    JP NZ, ERROR_EXIT

    ; Valida o Acumulador (CPI NUNCA deve alterar A)
    LD A, D
    CP H
    JP NZ, ERROR_EXIT

    ; =========================================================
    ; PASSO 5: Incrementar Loops (E -> D -> BC -> C)
    ; =========================================================
    INC E               
    JR NZ, LOOP_MEM     

    INC D               
    JR NZ, LOOP_A       

    LD A, [BC_VAL]
    INC A
    LD [BC_VAL], A
    CP 0x03             ; Se chegou a 3, testou BC=1 e BC=2
    JR NZ, LOOP_BC

    LD A, [C_FLAG]
    INC A
    LD [C_FLAG], A
    CP 0x02             ; Se chegou a 2, testou C=0 e C=1
    JP NZ, LOOP_C

    ; SUCESSO ABSOLUTO
    LD IX, STR_OK
    CALL PRINT_STRING
    CALL PRINT_CRLF
    RST 0x00


; =========================================================
; A "MAGIA NEGRA" DAS FLAGS DO CPI
; =========================================================
CALC_EXPECTED_FLAGS:
    ; 1. Usa o CP normal para pegar as flags base (S, Z, H, N=1)
    LD A, D
    CP E
    PUSH AF
    POP HL              ; L tem as flags base
    LD A, L
    AND 0xD2            ; Máscara 1101 0010 (Isola S, Z, H, N)
    LD [EXP_F], A       ; Salva as flags base

    ; 2. Regra da Paridade/Overflow (P/V reflete BC - 1 != 0)
    LD A, [BC_VAL]
    DEC A
    JR Z, SKIP_PV       ; Se BC zerou, P/V fica 0
    LD A, [EXP_F]
    OR 0x04             ; Seta a flag P/V (Bit 2)
    LD [EXP_F], A
SKIP_PV:

    ; 3. Flags F3 e F5 (A fórmula secreta: tmp = A - [HL] - H_Flag)
    LD A, D
    SUB E               ; A = A - [HL]
    BIT 4, L            ; Checa o Half-Carry da subtração original
    JR Z, SKIP_H_SUB
    DEC A               ; Subtrai o H_Flag
SKIP_H_SUB:
    LD B, A             ; B agora contém o "tmp"

    ; Extrai Bit 3 do tmp para a Flag 3
    LD A, B
    AND 0x08
    LD C, A
    LD A, [EXP_F]
    OR C
    LD [EXP_F], A

    ; Extrai Bit 1 do tmp e joga na Flag 5 (Shift Left x4)
    LD A, B
    AND 0x02
    ADD A, A
    ADD A, A
    ADD A, A
    ADD A, A            ; Bit 1 virou Bit 5 (0x20)
    LD C, A
    LD A, [EXP_F]
    OR C
    LD [EXP_F], A

    ; 4. Preserva a Flag de Carry original
    LD A, [C_FLAG]
    AND 0x01
    LD C, A
    LD A, [EXP_F]
    OR C
    LD [EXP_F], A

    RET

; =========================================================
; TRATAMENTO DE ERRO
; =========================================================
ERROR_EXIT:
    LD IX, STR_IN
    CALL PRINT_STRING
    LD A, D             ; Old A
    CALL PRINT_HEX8
    
    LD IX, STR_MEM
    CALL PRINT_STRING
    LD A, E             ; [HL]
    CALL PRINT_HEX8
    
    LD IX, STR_BC
    CALL PRINT_STRING
    LD A, [BC_VAL]      ; BC Input
    CALL PRINT_HEX8

    LD IX, STR_CFLG
    CALL PRINT_STRING
    LD A, [C_FLAG]      ; C Input
    CALL PRINT_HEX8
    
    LD IX, STR_EXP
    CALL PRINT_STRING
    LD A, [EXP_F]       ; Expected F
    CALL PRINT_HEX8
    
    LD IX, STR_FND
    CALL PRINT_STRING
    LD A, L             ; Actual F
    CALL PRINT_HEX8
    
    CALL PRINT_CRLF
    RST 0x00

; =========================================================
; ROTINAS DE IMPRESSÃO (BDOS)
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
    ; Cai pro PRINT_CHAR

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
; VARIÁVEIS NA MEMÓRIA
; =========================================================
C_FLAG:   DB 0x00
BC_VAL:   DB 0x00
EXP_F:    DB 0x00

; =========================================================
; STRINGS
; =========================================================
STR_OK:   DB "OK! TESTE CPI FINALIZADO. FLAGS PERFEITAS.", 0
STR_IN:   DB "IN A:", 0
STR_MEM:  DB " [HL]:", 0
STR_BC:   DB " BC:", 0
STR_CFLG: DB " C:", 0
STR_EXP:  DB " | EXP F:", 0
STR_FND:  DB " | FND F:", 0