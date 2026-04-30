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

bool cf_get();
void cf_set(bool value);
bool yf_get();
void yf_set(bool value);
bool xf_get();
void xf_set(bool value);
bool nf_get();
void nf_set(bool value);
bool pvf_get();
void pvf_set(bool value);
bool hf_get();
void hf_set(bool value);
bool sf_get();
void sf_set(bool value);
bool zf_get();
void zf_set(bool value);


// --== mem.c ==--

uint8_t mem_get_byte(uint16_t address);
void mem_set_byte(uint16_t address, uint8_t value);
uint16_t mem_get_word(uint16_t address);
void mem_set_word(uint16_t address, uint16_t value);
uint8_t ip_get_byte();
int8_t ip_get_byte_signed();
uint16_t ip_get_word();

// --== screen.c ==--


#define VDP_MEMORY_MAX 0x20000
extern char _vdp_memory[VDP_MEMORY_MAX];
char printable(char c);
void screen_draw();
void screen_draw_if_changed();
void screen_clear();
void screen_goto(int line, int column);
void screen_put_char(char c);
void screen_out_99(uint8_t value);
uint8_t screen_in_99();
void screen_out_98(uint8_t value);
uint8_t screen_in_98();

// --== keyb.c ==--

void keyb_init();
void keyb_exit();
void keyb_process();
bool keyb_pop(char *out);
char keyb_wait_pop();

// --== alu.c ==--

uint8_t alu_inc_byte(uint8_t value);
uint8_t alu_dec_byte(uint8_t value);
uint8_t alu_rlca(uint8_t value);
uint8_t alu_rlc(uint8_t value);
uint8_t alu_rrca(uint8_t value);
uint8_t alu_rrc(uint8_t value);
uint8_t alu_sla(uint8_t value);
uint8_t alu_sra(uint8_t value);
uint8_t alu_sll(uint8_t value);
uint8_t alu_srl(uint8_t value);
uint8_t alu_rla(uint8_t value);
uint8_t alu_rra(uint8_t value);
uint8_t alu_rl(uint8_t value);
uint8_t alu_rr(uint8_t value);
uint16_t alu_add_word(uint16_t value1, uint16_t value2);
uint8_t alu_add_byte(uint8_t value1, uint8_t value2);
uint16_t alu_adc_word(uint16_t value1, uint16_t value2);
uint8_t alu_adc_byte(uint8_t value1, uint8_t value2);
uint8_t alu_sub_byte(uint8_t value1, uint8_t value2);
uint8_t alu_cp_byte(uint8_t value1, uint8_t value2);
uint8_t alu_cp_extended_byte(uint8_t value1, uint8_t value2);
uint8_t alu_neg_byte(uint8_t value);
uint8_t alu_sbc_byte(uint8_t value1, uint8_t value2);
uint16_t alu_sbc_word(uint16_t value1, uint16_t value2);
uint8_t alu_and_byte(uint8_t value1, uint8_t value2);
void alu_bit(uint8_t value, uint8_t bit);
uint8_t alu_xor_byte(uint8_t value1, uint8_t value2);
uint8_t alu_or_byte(uint8_t value1, uint8_t value2);
void alu_parity(uint8_t value);
uint8_t alu_daa(uint8_t value);

// --== exec.c ==--

void exec();

// --== hardware.c ==--

void hardware_out(uint8_t port, uint8_t value);
uint8_t hardware_in(uint8_t port);

// --== abi_dos.c ==--

void abi_dos_call_5();

// --== disasm.c ==--

extern char _disasm[256];
void disasm(uint16_t address);

// --== abi_msxbios.c ==--

void abi_msxbios_reboot();
void abi_msxbios_rdslt();
void abi_msxbios_wrslt();
void abi_msxbios_callslt();
void abi_msxbios_enaslt();
void abi_msxbios_callf();
