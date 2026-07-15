// [English] BSD 4-Clause License
// [Portuguese] Licenca BSD 4-Clausulas
// 
// [English] Copyright (c) 2025,2026, Humberto Costa dos Santos Junior
// [Portuguese] Copyright (c) 2025,2026, Humberto Costa dos Santos Junior
// [English] All rights reserved.
// [Portuguese] Todos os direitos reservados.

#pragma once

// [English] --== headers ==--
// [Portuguese] --== headers ==--

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

// [English] --== common ==--
// [Portuguese] --== common ==--

#pragma pack(1)

// [English] Record type enumeration for the object file format
// [Portuguese] Enumeracao dos tipos de registro para o formato de arquivo objeto
typedef enum rectype_t
{
    // [English] File info records
    // [Portuguese] Registros de informacao de arquivo
    // [English] DATA = FILENAME
    // [Portuguese] DATA = NOME_DO_ARQUIVO
    REC_FILENAME,
    // [English] VALUE = LINE, AUX = COLUMN
    // [Portuguese] VALUE = LINHA, AUX = COLUNA
    REC_POSITION,
    // [English] DATA = FILENAME
    // [Portuguese] DATA = NOME_DO_ARQUIVO
    REC_END_OF_FILE,

    // [English] Section info records
    // [Portuguese] Registros de informacao de secao
    REC_SECTION_TEXT = 0x10,
    REC_SECTION_DATA,
    REC_SECTION_BSS,
    REC_SECTION_RELOC,

    // [English] Data records
    // [Portuguese] Registros de dados
    REC_DATA = 0x20,
    // [English] VALUE = SIZE
    // [Portuguese] VALUE = TAMANHO
    REC_DATA_RESERVE,

    // [English] Constant declaration records
    // [Portuguese] Registros de declaracao de constante
    // [English] DATA = NAME
    // [Portuguese] DATA = NOME
    REC_CONST_LABEL = 0x30,
    // [English] DATA = NAME, VALUE = INITIAL VALUE (signed int16)
    // [Portuguese] DATA = NOME, VALUE = VALOR INICIAL (int16 com sinal)
    REC_CONST_CUSTOM,
    // [English] DATA = NAME, VALUE = UNSIGNED INITIAL VALUE (uint16 - avoids sign extension for 0x8000..0xFFFF)
    // [Portuguese] DATA = NOME, VALUE = VALOR INICIAL SEM SINAL (uint16 - evita extensão de sinal para 0x8000..0xFFFF)
    REC_CONST_CUSTOM_UNSIGNED,
    // [English] DATA = NAME
    // [Portuguese] DATA = NOME
    REC_CONST_AS_GLOBAL_LABEL,

    // [English] Expression records
    // [Portuguese] Registros de expressao
    REC_EXPR_RESET = 0x40,
    // [English] VALUE = VALUE
    // [Portuguese] VALUE = VALOR
    REC_EXPR_PUSH_VALUE,
    // [English] VALUE = UNSIGNED VALUE
    // [Portuguese] VALUE = VALOR SEM SINAL
    REC_EXPR_PUSH_VALUE_UNSIGNED,
    // [English] CURRENT OFFSET + VALUE (OFFSET)
    // [Portuguese] OFFSET ATUAL + VALUE (OFFSET)
    REC_EXPR_PUSH_OFFSET,
    // [English] DATA = CONSTANT NAME
    // [Portuguese] DATA = NOME DA CONSTANTE
    REC_EXPR_PUSH_CONST,
    // [English] DATA = LABEL NAME
    // [Portuguese] DATA = NOME DO LABEL
    REC_EXPR_PUSH_SEGMENT,

    // [English] Arithmetic operations
    // [Portuguese] Operacoes aritmeticas
    REC_EXPR_ADD = 0x50,
    REC_EXPR_SUB,
    REC_EXPR_MUL,
    REC_EXPR_DIV,
    REC_EXPR_MOD,
    REC_EXPR_SHL,
    REC_EXPR_SHR,
    REC_EXPR_NOT,
    REC_EXPR_AND,
    REC_EXPR_OR,
    REC_EXPR_XOR,

    // [English] Expression output operations
    // [Portuguese] Operacoes de saida de expressao
    REC_EXPR_POP_TO_CONST = 0x60,
    REC_EXPR_POP_INT8_EMIT,
    REC_EXPR_POP_INT16_EMIT,
    REC_EXPR_POP_INT16_RELOCATABLE_EMIT,
    REC_EXPR_POP_REPEAT_TIMES,
    REC_EXPR_REPEAT_TIMES_END,

    // [English] CPU identification records
    // [Portuguese] Registros de identificacao de CPU
    REC_CPU = 0xf0,
    // [English] intel 8080
    // [Portuguese] intel 8080
    REC_CPU_8080 = 0xf0,
    // [English] intel 8085
    // [Portuguese] intel 8085
    REC_CPU_8085,
    // [English] zilog z80
    // [Portuguese] zilog z80
    REC_CPU_Z80,
    // [English] intel 8086
    // [Portuguese] intel 8086
    REC_CPU_8086,
    MASK_REC_TYPE = 0xf,
} rectype_t;

// [English] Record header structure (8 bytes)
// [Portuguese] Estrutura do cabecalho do registro (8 bytes)
typedef struct record_header_t
{
    // [English] Record type
    // [Portuguese] Tipo do registro
    uint8_t type;
    // [English] Size of data field
    // [Portuguese] Tamanho do campo de dados
    uint8_t data_size;
    // [English] Value field
    // [Portuguese] Campo de valor
    int16_t value;
    // [English] Auxiliary field
    // [Portuguese] Campo auxiliar
    uint16_t aux;
} record_header_t;

// [English] Full record with header and data payload
// [Portuguese] Registro completo com cabecalho e payload de dados
typedef struct record_t
{
    // [English] Record header
    // [Portuguese] Cabecalho do registro
    record_header_t header;
    // [English] Data payload
    // [Portuguese] Payload de dados
    uint8_t data[256];
} record_t;

#pragma pack()
