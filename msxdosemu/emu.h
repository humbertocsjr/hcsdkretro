// BSD 4-Clause License
// 
// Copyright (c) 2025,2026, Humberto Costa dos Santos Junior
// All rights reserved.

#pragma once

// --== headers ==--

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include "../include/version.h"

/* ── Platform abstraction ─────────────────────────────────────── */

#ifdef _WIN32
  #include <windows.h>
  #include <conio.h>
  #include <io.h>
  #include <direct.h>
  #include <sys/stat.h>
  #include "win32_dirent.h"
  #define PATHSEP '\\'
  #define PATHSEPSTR "\\"
  #define mkdir(p, m) _mkdir(p)
  #define RMDIR _rmdir
  #define fileno _fileno
  #define ftruncate _chsize_s
  #ifndef STDIN_FILENO
  #define STDIN_FILENO 0
  #endif
#else
  #include <termios.h>
  #include <unistd.h>
  #include <dirent.h>
  #include <sys/stat.h>
  #include <sys/statvfs.h>
  #define PATHSEP '/'
  #define PATHSEPSTR "/"
  #define RMDIR rmdir
#endif

// --== common ==--

#pragma pack(1)
typedef struct abi_dos_fcb_t
{
    uint8_t drive;
    char name[8];
    char ext[3];
    uint8_t extent_low;
    uint8_t attributes;
    uint8_t extent_high;
    uint8_t record_count;
    uint32_t file_size;
    uint32_t volume_id;
    uint8_t internal[8];
    uint8_t current_record;
    uint32_t random_record_number;
} abi_dos_fcb_t;

typedef struct abi_dos_fib_t
{
    uint8_t zero;
    char name[8];
    char ext[3];
    uint8_t attribute;
    uint16_t time;
    uint16_t date;
    uint16_t start_cluster;
    uint32_t file_size;
    uint8_t logical_drive;
    uint8_t reserved[37];
} abi_dos_fib_t;

typedef struct z80_regs_t
{
    union
    {
        uint16_t word;
        struct
        {
            uint8_t f;
            uint8_t a;
        };
    } af;
    uint16_t af_alt;
    union
    {
        uint16_t word;
        struct
        {
            uint8_t c;
            uint8_t b;
        };
    } bc;
    uint16_t bc_alt;
    union
    {
        uint16_t word;
        struct
        {
            uint8_t e;
            uint8_t d;
        };
    } de;
    uint16_t de_alt;
    union
    {
        uint16_t word;
        struct
        {
            uint8_t l;
            uint8_t h;
        };
    } hl;
    uint16_t hl_alt;
    union
    {
        uint16_t word;
        struct
        {
            uint8_t l;
            uint8_t h;
        };
    } ix;
    union
    {
        uint16_t word;
        struct
        {
            uint8_t l;
            uint8_t h;
        };
    } iy;
    uint16_t sp;
    uint16_t ip;
    uint8_t i;
    uint8_t r;
    uint16_t value;
    bool prefix_dd;
    bool prefix_fd;
    bool prefix_cb;
    bool prefix_ed;
    uint8_t interrupt_mode;
    bool interrupts;
} z80_regs_t;

#pragma pack()

// --== emu.c ==--


extern bool _debuggable;
extern bool _debug;
extern bool _trace;
extern uint8_t _memory[0x100ff];
extern z80_regs_t _regs_curr;
extern z80_regs_t _regs_prev;
extern bool _executing;
extern bool _next_step;
extern bool _skip_call_step;
extern int32_t _skip_call_step_address;
extern char *_disk_a_path;
extern char *_disk_b_path;
extern uint16_t _disk_transferr_address;
extern int _return_code;
extern bool keyb_avail();

// --== flags.c ==--

// [English] Get/Set Carry Flag
// [Portuguese] Obtém/Define a flag de Carry
bool cf_get();
void cf_set(bool value);
// [English] Get/Set Y Flag (bit 5)
// [Portuguese] Obtém/Define a flag Y
bool yf_get();
void yf_set(bool value);
// [English] Get/Set X Flag (bit 3)
// [Portuguese] Obtém/Define a flag X
bool xf_get();
void xf_set(bool value);
// [English] Get/Set Subtract Flag (N, bit 1)
// [Portuguese] Obtém/Define a flag N
bool nf_get();
void nf_set(bool value);
// [English] Get/Set Parity/Overflow Flag (P/V, bit 2)
// [Portuguese] Obtém/Define a flag P/V
bool pvf_get();
void pvf_set(bool value);
// [English] Get/Set Half-Carry Flag (H, bit 4)
// [Portuguese] Obtém/Define a flag H
bool hf_get();
void hf_set(bool value);
// [English] Get/Set Sign Flag (S, bit 7)
// [Portuguese] Obtém/Define a flag S
bool sf_get();
void sf_set(bool value);
// [English] Get/Set Zero Flag (Z, bit 6)
// [Portuguese] Obtém/Define a flag Z
bool zf_get();
void zf_set(bool value);


// --== mem.c ==--

// [English] Read/write bytes and words from emulated memory
// [Portuguese] Lê/escreve bytes e words da memória emulada
uint8_t mem_get_byte(uint16_t address);
void mem_set_byte(uint16_t address, uint8_t value);
uint16_t mem_get_word(uint16_t address);
void mem_set_word(uint16_t address, uint16_t value);
// [English] Fetch unsigned byte, signed byte, or word from instruction pointer
// [Portuguese] Busca byte unsigned, byte signed ou word do ponteiro de instrução
uint8_t ip_get_byte();
int8_t ip_get_byte_signed();
uint16_t ip_get_word();

// --== screen.c ==--


#define VDP_MEMORY_MAX 0x20000
extern char _vdp_memory[VDP_MEMORY_MAX];
// [English] Return printable character or space
// [Portuguese] Retorna caractere imprimível ou espaço
char printable(char c);
// [English] Initialize screen subsystem
// [Portuguese] Inicializa subsistema de tela
void screen_init();
// [English] Full debug screen redraw
// [Portuguese] Redesenho completo da tela de debug
void screen_draw();
// [English] Redraw debug screen only if changed
// [Portuguese] Redesenha tela de debug apenas se alterada
void screen_draw_if_changed();
// [English] Clear the screen
// [Portuguese] Limpa a tela
void screen_clear();
// [English] Move cursor to position (1-based)
// [Portuguese] Move cursor para posição (base 1)
void screen_goto(int line, int column);
// [English] Output a character to the screen
// [Portuguese] Envia caractere para a tela
void screen_put_char(char c);
// [English] VDP port 0x99 write (set register)
// [Portuguese] Escrita na porta VDP 0x99 (registrador)
void screen_out_99(uint8_t value);
// [English] VDP port 0x99 read (status)
// [Portuguese] Leitura da porta VDP 0x99 (status)
uint8_t screen_in_99();
// [English] VDP port 0x98 write (VRAM data)
// [Portuguese] Escrita na porta VDP 0x98 (dados VRAM)
void screen_out_98(uint8_t value);
// [English] VDP port 0x98 read (VRAM data)
// [Portuguese] Leitura da porta VDP 0x98 (dados VRAM)
uint8_t screen_in_98();

// --== keyb.c ==--

// [English] Initialize keyboard input
// [Portuguese] Inicializa entrada do teclado
void keyb_init();
// [English] Restore console settings on exit
// [Portuguese] Restaura configs do console na saída
void keyb_exit();
// [English] Process keyboard input (keys + debugger commands)
// [Portuguese] Processa entrada do teclado (teclas + comandos do debugger)
void keyb_process();
// [English] Pop a key from the circular buffer
// [Portuguese] Remove tecla do buffer circular
bool keyb_pop(char *out);
// [English] Wait for and pop a key (blocking)
// [Portuguese] Aguarda e remove tecla (bloqueante)
char keyb_wait_pop();

// --== alu.c ==--

// [English] Increment byte
// [Portuguese] Incrementa byte
uint8_t alu_inc_byte(uint8_t value);
// [English] Decrement byte
// [Portuguese] Decrementa byte
uint8_t alu_dec_byte(uint8_t value);
// [English] Rotate left circular accumulator
// [Portuguese] Rotaciona à esquerda circular acumulador
uint8_t alu_rlca(uint8_t value);
// [English] Rotate left circular
// [Portuguese] Rotaciona à esquerda circular
uint8_t alu_rlc(uint8_t value);
// [English] Rotate right circular accumulator
// [Portuguese] Rotaciona à direita circular acumulador
uint8_t alu_rrca(uint8_t value);
// [English] Rotate right circular
// [Portuguese] Rotaciona à direita circular
uint8_t alu_rrc(uint8_t value);
// [English] Shift left arithmetic
// [Portuguese] Deslocamento aritmético à esquerda
uint8_t alu_sla(uint8_t value);
// [English] Shift right arithmetic
// [Portuguese] Deslocamento aritmético à direita
uint8_t alu_sra(uint8_t value);
// [English] Shift left logical
// [Portuguese] Deslocamento lógico à esquerda
uint8_t alu_sll(uint8_t value);
// [English] Shift right logical
// [Portuguese] Deslocamento lógico à direita
uint8_t alu_srl(uint8_t value);
// [English] Rotate left through carry (accumulator)
// [Portuguese] Rotaciona à esquerda via carry
uint8_t alu_rla(uint8_t value);
// [English] Rotate right through carry (accumulator)
// [Portuguese] Rotaciona à direita via carry
uint8_t alu_rra(uint8_t value);
// [English] Rotate left through carry
// [Portuguese] Rotaciona à esquerda via carry
uint8_t alu_rl(uint8_t value);
// [English] Rotate right through carry
// [Portuguese] Rotaciona à direita via carry
uint8_t alu_rr(uint8_t value);
// [English] Add 16-bit word
// [Portuguese] Soma word de 16 bits
uint16_t alu_add_word(uint16_t value1, uint16_t value2);
// [English] Add byte
// [Portuguese] Soma byte
uint8_t alu_add_byte(uint8_t value1, uint8_t value2);
// [English] Add 16-bit word with carry
// [Portuguese] Soma word 16 bits com carry
uint16_t alu_adc_word(uint16_t value1, uint16_t value2);
// [English] Add byte with carry
// [Portuguese] Soma byte com carry
uint8_t alu_adc_byte(uint8_t value1, uint8_t value2);
// [English] Subtract byte
// [Portuguese] Subtrai byte
uint8_t alu_sub_byte(uint8_t value1, uint8_t value2);
// [English] Compare byte
// [Portuguese] Compara byte
uint8_t alu_cp_byte(uint8_t value1, uint8_t value2);
// [English] Extended compare for CPI/CPD
// [Portuguese] Comparação estendida para CPI/CPD
uint8_t alu_cp_extended_byte(uint8_t value1, uint8_t value2);
// [English] Negate accumulator
// [Portuguese] Nega o acumulador
uint8_t alu_neg_byte(uint8_t value);
// [English] Subtract byte with carry
// [Portuguese] Subtrai byte com carry
uint8_t alu_sbc_byte(uint8_t value1, uint8_t value2);
// [English] Subtract 16-bit word with carry
// [Portuguese] Subtrai word 16 bits com carry
uint16_t alu_sbc_word(uint16_t value1, uint16_t value2);
// [English] Logical AND byte
// [Portuguese] AND lógico entre bytes
uint8_t alu_and_byte(uint8_t value1, uint8_t value2);
// [English] Test bit
// [Portuguese] Testa um bit
void alu_bit(uint8_t value, uint8_t bit);
// [English] Logical XOR byte
// [Portuguese] XOR lógico entre bytes
uint8_t alu_xor_byte(uint8_t value1, uint8_t value2);
// [English] Logical OR byte
// [Portuguese] OR lógico entre bytes
uint8_t alu_or_byte(uint8_t value1, uint8_t value2);
// [English] Calculate parity
// [Portuguese] Calcula paridade
void alu_parity(uint8_t value);
// [English] Decimal adjust accumulator
// [Portuguese] Ajuste decimal do acumulador
uint8_t alu_daa(uint8_t value);

// --== exec.c ==--

// [English] Main execution loop
// [Portuguese] Loop principal de execução
void exec();

// --== hardware.c ==--

// [English] Write to MSX I/O port
// [Portuguese] Escreve em porta I/O do MSX
void hardware_out(uint8_t port, uint8_t value);
// [English] Read from MSX I/O port
// [Portuguese] Lê de porta I/O do MSX
uint8_t hardware_in(uint8_t port);

// --== abi_dos.c ==--

// [English] MSX-DOS CALL 5 entry point (BDOS system calls)
// [Portuguese] Ponto de entrada CALL 5 do MSX-DOS (chamadas de sistema BDOS)
void abi_dos_call_5();

// --== disasm.c ==--

extern char _disasm[256];
// [English] Disassemble instruction at address
// [Portuguese] Desmonta instrução no endereço
void disasm(uint16_t address);

// --== abi_msxbios.c ==--

// [English] MSX BIOS reboot (HALT)
// [Portuguese] Reinicialização via BIOS MSX (HALT)
void abi_msxbios_reboot();
// [English] MSX BIOS Read Slot
// [Portuguese] Leitura de slot da BIOS MSX
void abi_msxbios_rdslt();
// [English] MSX BIOS Write Slot
// [Portuguese] Escrita em slot da BIOS MSX
void abi_msxbios_wrslt();
// [English] MSX BIOS Call Slot
// [Portuguese] Chamada de slot da BIOS MSX
void abi_msxbios_callslt();
// [English] MSX BIOS Enable Slot
// [Portuguese] Habilitar slot da BIOS MSX
void abi_msxbios_enaslt();
// [English] MSX BIOS Call Far
// [Portuguese] Chamada far da BIOS MSX
void abi_msxbios_callf();
