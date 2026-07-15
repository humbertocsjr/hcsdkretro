/**
 * ELF32 + DWARF4 Debug Info Generation
 * Generates debuggable ELF32 executables compatible with GDB/LLDB/QEMU
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../include/obj.h"
#include "link.h"

/* ELF32 Constants */
#define EI_MAG0         0
#define EI_MAG1         1
#define EI_MAG2         2
#define EI_MAG3         3
#define EI_CLASS        4
#define EI_DATA         5
#define EI_VERSION      6
#define EI_OSABI        7
#define EI_ABIVERSION   8
#define EI_PAD          9
#define EI_NIDENT      16

#define ELFCLASS32      1
#define ELFDATA2LSB     1
#define ELFOSABI_SYSV   0

#define ET_EXEC         2
#define EM_386          3

#define PT_LOAD         1
#define PF_X            1
#define PF_W            2
#define PF_R            4

#define SHT_NULL        0
#define SHT_PROGBITS    1
#define SHT_SYMTAB      2
#define SHT_STRTAB      3
#define SHT_REL         9

#define SHF_WRITE       1
#define SHF_ALLOC       2
#define SHF_EXECINSTR   4

#define STT_OBJECT      1
#define STT_FUNC        2
#define STB_LOCAL       0
#define STB_GLOBAL      1

/* DWARF4 Constants */
#define DW_TAG_compile_unit     0x11
#define DW_TAG_label            0x0a
#define DW_TAG_variable         0x05
#define DW_TAG_base_type        0x24
#define DW_TAG_array_type       0x01
#define DW_TAG_subrange_type    0x21

#define DW_AT_producer          0x25
#define DW_AT_language          0x13
#define DW_AT_name              0x03
#define DW_AT_low_pc            0x11
#define DW_AT_high_pc           0x12
#define DW_AT_stmt_list         0x10
#define DW_AT_location          0x02
#define DW_AT_type              0x49
#define DW_AT_byte_size         0x0b
#define DW_AT_encoding          0x0e
#define DW_AT_upper_bound       0x2f

#define DW_FORM_addr            0x01
#define DW_FORM_data1           0x0b
#define DW_FORM_data2           0x05
#define DW_FORM_data4           0x06
#define DW_FORM_string          0x08
#define DW_FORM_strp            0x0e
#define DW_FORM_ref1            0x11
#define DW_FORM_ref4            0x13

#define DW_OP_addr              0x03
#define DW_OP_const4u           0x0f

#define DW_ATE_unsigned         0x07
#define DW_ATE_signed           0x05
#define DW_ATE_unsigned_char    0x08

#define DW_LNS_extended_op      0x00
#define DW_LNS_copy             0x01
#define DW_LNS_advance_pc       0x02
#define DW_LNS_advance_line     0x03
#define DW_LNE_end_sequence     0x01
#define DW_LNE_set_address      0x02
#define DW_LNE_define_file      0x04

#define DW_LANG_C               0x02

/* Variable size categories for type declaration */
typedef struct {
    uint32_t address;
    uint32_t size;
    const char *name;
    bool is_global;
    rectype_t section;
} var_info_t;

/* Helper: Write LEB128 unsigned */
static int write_uleb128(FILE *f, uint32_t val) {
    int count = 0;
    do {
        uint8_t b = val & 0x7f;
        val >>= 7;
        if (val != 0) b |= 0x80;
        fputc(b, f);
        count++;
    } while (val != 0);
    return count;
}

/* Helper: Write LEB128 signed */
static int write_sleb128(FILE *f, int32_t val) {
    int count = 0;
    while (1) {
        uint8_t b = val & 0x7f;
        val >>= 7;
        if ((val == 0 && (b & 0x40) == 0) || (val == -1 && (b & 0x40) != 0)) {
            fputc(b, f);
            count++;
            break;
        }
        fputc(b | 0x80, f);
        count++;
    }
    return count;
}

/* Helper: Write 32-bit little-endian */
static void write_u32(FILE *f, uint32_t val) {
    fputc(val & 0xff, f);
    fputc((val >> 8) & 0xff, f);
    fputc((val >> 16) & 0xff, f);
    fputc((val >> 24) & 0xff, f);
}

/* Helper: Write 16-bit little-endian */
static void write_u16(FILE *f, uint16_t val) {
    fputc(val & 0xff, f);
    fputc((val >> 8) & 0xff, f);
}

/* Helper: Write 8-bit */
static void write_u8(FILE *f, uint8_t val) {
    fputc(val, f);
}

/**
 * Write ELF32 Header
 */
static void elf_write_header(FILE *f, uint32_t entry, uint32_t ph_offset, 
                              uint32_t sh_offset, uint16_t sh_num, uint16_t sh_strndx) {
    /* e_ident[EI_NIDENT] */
    write_u8(f, 0x7f);               /* EI_MAG0 */
    fputc('E', f); fputc('L', f); fputc('F', f);  /* EI_MAG1-3 */
    write_u8(f, ELFCLASS32);         /* EI_CLASS */
    write_u8(f, ELFDATA2LSB);        /* EI_DATA */
    write_u8(f, 1);                  /* EI_VERSION */
    write_u8(f, ELFOSABI_SYSV);      /* EI_OSABI */
    write_u8(f, 0);                  /* EI_ABIVERSION */
    for (int i = 0; i < 7; i++) write_u8(f, 0);  /* padding */

    write_u16(f, ET_EXEC);           /* e_type */
    write_u16(f, EM_386);            /* e_machine */
    write_u32(f, 1);                 /* e_version */
    write_u32(f, entry);             /* e_entry */
    write_u32(f, ph_offset);         /* e_phoff */
    write_u32(f, sh_offset);         /* e_shoff */
    write_u32(f, 0);                 /* e_flags */
    write_u16(f, 52);                /* e_ehsize */
    write_u16(f, 32);                /* e_phentsize */
    write_u16(f, 1);                 /* e_phnum */
    write_u16(f, 40);                /* e_shentsize */
    write_u16(f, sh_num);            /* e_shnum */
    write_u16(f, sh_strndx);         /* e_shstrndx */
}

/**
 * Write Program Header (PT_LOAD)
 */
static void elf_write_program_header(FILE *f, uint32_t text_addr, uint32_t filesz) {
    write_u32(f, PT_LOAD);           /* p_type */
    write_u32(f, 52 + 32);           /* p_offset = after headers */
    write_u32(f, text_addr);         /* p_vaddr */
    write_u32(f, text_addr);         /* p_paddr */
    write_u32(f, filesz);            /* p_filesz */
    write_u32(f, filesz);            /* p_memsz */
    write_u32(f, PF_R | PF_X);       /* p_flags = R+X */
    write_u32(f, 0x1000);            /* p_align */
}

/**
 * Build string table from filenames and symbol names
 * Returns offset in strtab after insertion
 */
struct strtab_entry {
    const char *str;
    uint32_t offset;
};

static struct strtab_entry *strtab_entries = NULL;
static int strtab_count = 0;

static uint32_t strtab_add(const char *str) {
    if (str == NULL) str = "";
    
    /* Check if already exists */
    for (int i = 0; i < strtab_count; i++) {
        if (strcmp(strtab_entries[i].str, str) == 0) {
            return strtab_entries[i].offset;
        }
    }
    
    /* Add new entry */
    strtab_entries = realloc(strtab_entries, (strtab_count + 1) * sizeof(*strtab_entries));
    strtab_entries[strtab_count].str = str;
    strtab_entries[strtab_count].offset = 0;  /* Will be calculated */
    strtab_count++;
    return 0;  /* Temp; will be fixed in second pass */
}

static uint32_t strtab_finalize(FILE *f) {
    uint32_t offset = 0;
    
    /* First pass: calculate offsets */
    for (int i = 0; i < strtab_count; i++) {
        strtab_entries[i].offset = offset;
        offset += strlen(strtab_entries[i].str) + 1;
    }
    
    /* Second pass: write strings */
    for (int i = 0; i < strtab_count; i++) {
        fputs(strtab_entries[i].str, f);
        fputc('\0', f);
    }
    
    return offset;
}

/**
 * Collect variables from data and bss sections
 * Calculates size from address gaps between consecutive variables
 */
static var_info_t *var_list = NULL;
static int var_count = 0;

static void collect_variables(void) {
    /* Collect all DATA and BSS symbols */
    const_t *c = _consts;
    while (c != NULL) {
        if (c->section == REC_SECTION_DATA || c->section == REC_SECTION_BSS) {
            var_list = realloc(var_list, (var_count + 1) * sizeof(var_info_t));
            var_list[var_count].address = c->value;
            var_list[var_count].name = c->name;
            var_list[var_count].is_global = c->is_global;
            var_list[var_count].section = c->section;
            var_list[var_count].size = 0;  /* Will calculate */
            var_count++;
        }
        c = c->next;
    }
    
    /* Sort by section then address */
    for (int i = 0; i < var_count - 1; i++) {
        for (int j = i + 1; j < var_count; j++) {
            int cmp_section = (var_list[i].section < var_list[j].section) ? -1 : 
                            (var_list[i].section > var_list[j].section) ? 1 : 0;
            if (cmp_section > 0 || 
                (cmp_section == 0 && var_list[i].address > var_list[j].address)) {
                var_info_t tmp = var_list[i];
                var_list[i] = var_list[j];
                var_list[j] = tmp;
            }
        }
    }
    
    /* Calculate sizes from address gaps */
    for (int i = 0; i < var_count; i++) {
        if (i + 1 < var_count && var_list[i].section == var_list[i+1].section) {
            /* Size = next address - current address */
            var_list[i].size = var_list[i+1].address - var_list[i].address;
        } else {
            /* Last variable in section - assume minimum size */
            var_list[i].size = 1;
        }
        
        /* Minimum size is 1 byte */
        if (var_list[i].size == 0) {
            var_list[i].size = 1;
        }
    }
}

/**
 * Collect debug info and strings
 */
static void collect_debug_strings(void) {
    /* Add all filenames */
    file_info_t *f = _debug_files;
    while (f != NULL) {
        strtab_add(f->name);
        f = f->next;
    }
    
    /* Add all symbol names */
    const_t *c = _consts;
    while (c != NULL) {
        strtab_add(c->name);
        c = c->next;
    }
    
    strtab_add("producer");
    strtab_add("HC SDK Retro");
    strtab_add("language");
    strtab_add("uint8");
    strtab_add("uint16");
    strtab_add("uint32");
    strtab_add("byte");
    
    /* Add variable names from collected list */
    for (int i = 0; i < var_count; i++) {
        strtab_add(var_list[i].name);
    }
}

/**
 * Write DWARF4 Abbreviation Table (.debug_abbrev)
 * Defines the structure of DIE entries used in .debug_info
 */
static void elf_write_debug_abbrev(FILE *f, long *size) {
    long start = ftell(f);
    
    /* Abbreviation 1: DW_TAG_compile_unit */
    write_uleb128(f, 1);             /* abbrev code */
    write_uleb128(f, DW_TAG_compile_unit);
    write_u8(f, 1);                  /* children = yes */
    
    /* Attributes */
    write_uleb128(f, DW_AT_producer);
    write_uleb128(f, DW_FORM_strp);
    write_uleb128(f, DW_AT_language);
    write_uleb128(f, DW_FORM_data2);
    write_uleb128(f, DW_AT_name);
    write_uleb128(f, DW_FORM_strp);
    write_uleb128(f, DW_AT_low_pc);
    write_uleb128(f, DW_FORM_addr);
    write_uleb128(f, DW_AT_high_pc);
    write_uleb128(f, DW_FORM_addr);
    write_uleb128(f, DW_AT_stmt_list);
    write_uleb128(f, DW_FORM_data4);
    
    write_uleb128(f, 0);             /* end of attributes */
    write_uleb128(f, 0);             /* end of attribute spec */
    
    /* Abbreviation 2: DW_TAG_label */
    write_uleb128(f, 2);             /* abbrev code */
    write_uleb128(f, DW_TAG_label);
    write_u8(f, 0);                  /* children = no */
    
    write_uleb128(f, DW_AT_name);
    write_uleb128(f, DW_FORM_strp);
    write_uleb128(f, DW_AT_low_pc);
    write_uleb128(f, DW_FORM_addr);
    
    write_uleb128(f, 0);             /* end of attributes */
    write_uleb128(f, 0);             /* end of attribute spec */
    
    /* Abbreviation 3: DW_TAG_variable */
    write_uleb128(f, 3);             /* abbrev code */
    write_uleb128(f, DW_TAG_variable);
    write_u8(f, 0);                  /* children = no */
    
    write_uleb128(f, DW_AT_name);
    write_uleb128(f, DW_FORM_strp);
    write_uleb128(f, DW_AT_type);
    write_uleb128(f, DW_FORM_ref4);
    write_uleb128(f, DW_AT_location);
    write_uleb128(f, DW_FORM_data4);  /* Address offset */
    
    write_uleb128(f, 0);             /* end of attributes */
    write_uleb128(f, 0);             /* end of attribute spec */
    
    /* Abbreviation 4: DW_TAG_base_type (uint8) */
    write_uleb128(f, 4);             /* abbrev code */
    write_uleb128(f, DW_TAG_base_type);
    write_u8(f, 0);                  /* children = no */
    
    write_uleb128(f, DW_AT_name);
    write_uleb128(f, DW_FORM_strp);
    write_uleb128(f, DW_AT_byte_size);
    write_uleb128(f, DW_FORM_data1);
    write_uleb128(f, DW_AT_encoding);
    write_uleb128(f, DW_FORM_data1);
    
    write_uleb128(f, 0);             /* end of attributes */
    write_uleb128(f, 0);             /* end of attribute spec */
    
    /* Abbreviation 5: DW_TAG_base_type (uint16) */
    write_uleb128(f, 5);             /* abbrev code */
    write_uleb128(f, DW_TAG_base_type);
    write_u8(f, 0);                  /* children = no */
    
    write_uleb128(f, DW_AT_name);
    write_uleb128(f, DW_FORM_strp);
    write_uleb128(f, DW_AT_byte_size);
    write_uleb128(f, DW_FORM_data1);
    write_uleb128(f, DW_AT_encoding);
    write_uleb128(f, DW_FORM_data1);
    
    write_uleb128(f, 0);             /* end of attributes */
    write_uleb128(f, 0);             /* end of attribute spec */
    
    /* Abbreviation 6: DW_TAG_base_type (uint32) */
    write_uleb128(f, 6);             /* abbrev code */
    write_uleb128(f, DW_TAG_base_type);
    write_u8(f, 0);                  /* children = no */
    
    write_uleb128(f, DW_AT_name);
    write_uleb128(f, DW_FORM_strp);
    write_uleb128(f, DW_AT_byte_size);
    write_uleb128(f, DW_FORM_data1);
    write_uleb128(f, DW_AT_encoding);
    write_uleb128(f, DW_FORM_data1);
    
    write_uleb128(f, 0);             /* end of attributes */
    write_uleb128(f, 0);             /* end of attribute spec */
    
    /* Abbreviation 7: DW_TAG_array_type */
    write_uleb128(f, 7);             /* abbrev code */
    write_uleb128(f, DW_TAG_array_type);
    write_u8(f, 1);                  /* children = yes (for subrange) */
    
    write_uleb128(f, DW_AT_type);
    write_uleb128(f, DW_FORM_ref4);
    write_uleb128(f, DW_AT_byte_size);
    write_uleb128(f, DW_FORM_data1);
    
    write_uleb128(f, 0);             /* end of attributes */
    write_uleb128(f, 0);             /* end of attribute spec */
    
    /* Abbreviation 8: DW_TAG_subrange_type */
    write_uleb128(f, 8);             /* abbrev code */
    write_uleb128(f, DW_TAG_subrange_type);
    write_u8(f, 0);                  /* children = no */
    
    write_uleb128(f, DW_AT_upper_bound);
    write_uleb128(f, DW_FORM_data1);
    
    write_uleb128(f, 0);             /* end of attributes */
    write_uleb128(f, 0);             /* end of attribute spec */
    
    write_u8(f, 0);                  /* null abbreviation */
    
    *size = ftell(f) - start;
}

/**
 * Write DWARF4 Debug Info (.debug_info)
 */
static void elf_write_debug_info(FILE *f, uint32_t text_addr, uint32_t text_size,
                                  uint32_t abbrev_offset, uint32_t str_offset, 
                                  long *size) {
    long start = ftell(f);
    
    /* Unit length placeholder */
    long unit_len_pos = ftell(f);
    write_u32(f, 0);  /* Will be filled later */
    
    write_u16(f, 4);                 /* version = 4 */
    write_u32(f, abbrev_offset);     /* debug_abbrev_offset */
    write_u8(f, 4);                  /* address_size = 4 bytes */
    
    /* DIE 1: Compile Unit */
    write_uleb128(f, 1);             /* abbrev code = compile_unit */
    
    /* DW_AT_producer (strp) */
    write_u32(f, str_offset + 40);   /* offset in .debug_str */
    
    /* DW_AT_language (data2) */
    write_u16(f, DW_LANG_C);
    
    /* DW_AT_name (strp) */
    write_u32(f, str_offset + 60);   /* offset in .debug_str */
    
    /* DW_AT_low_pc (addr) */
    write_u32(f, text_addr);
    
    /* DW_AT_high_pc (addr) */
    write_u32(f, text_addr + text_size);
    
    /* DW_AT_stmt_list (data4) */
    write_u32(f, 0);                 /* line table offset */
    
    /* Save DIE offsets for type references */
    uint32_t base_type_uint8_offset = ftell(f) - start - 11; /* Approximate offset in compile unit */
    
    /* DIE 2-8: Base types */
    write_uleb128(f, 4);             /* abbrev = base_type uint8 */
    write_u32(f, str_offset + 80);   /* name: "uint8" */
    write_u8(f, 1);                  /* byte_size = 1 */
    write_u8(f, DW_ATE_unsigned);    /* encoding */
    
    write_uleb128(f, 5);             /* abbrev = base_type uint16 */
    write_u32(f, str_offset + 90);   /* name: "uint16" */
    write_u8(f, 2);                  /* byte_size = 2 */
    write_u8(f, DW_ATE_unsigned);
    
    write_uleb128(f, 6);             /* abbrev = base_type uint32 */
    write_u32(f, str_offset + 100);  /* name: "uint32" */
    write_u8(f, 4);                  /* byte_size = 4 */
    write_u8(f, DW_ATE_unsigned);
    
    write_uleb128(f, 4);             /* abbrev = base_type byte */
    write_u32(f, str_offset + 110);  /* name: "byte" */
    write_u8(f, 1);                  /* byte_size = 1 */
    write_u8(f, DW_ATE_unsigned_char);
    
    /* DIE children: Labels and Variables */
    /* First: line labels */
    int label_idx = 0;
    line_info_t *line = _debug_lines;
    while (line != NULL) {
        uint32_t addr = line->address + text_addr;
        uint16_t line_num = line->line;
        uint16_t col = line->column;
        
        /* Skip duplicates */
        if (line->next != NULL && line->address == line->next->address) {
            line = line->next;
            continue;
        }
        
        write_uleb128(f, 2);         /* abbrev code = label */
        
        /* DW_AT_name */
        static char label_name[32];
        snprintf(label_name, sizeof(label_name), "line_%u_%u", line_num, col);
        write_u32(f, str_offset + 120 + (label_idx * 20));
        
        /* DW_AT_low_pc */
        write_u32(f, addr);
        
        line = line->next;
        label_idx++;
    }
    
    /* Second: Variables from DATA and BSS sections */
    for (int i = 0; i < var_count; i++) {
        var_info_t *v = &var_list[i];
        
        write_uleb128(f, 3);         /* abbrev code = variable */
        
        /* DW_AT_name (strp) */
        uint32_t name_offset = str_offset + 200 + (i * 50);
        write_u32(f, name_offset);
        
        /* DW_AT_type (ref4) - offset to base type DIE */
        uint32_t type_offset;
        if (v->size == 1) {
            type_offset = base_type_uint8_offset + 11;
        } else if (v->size == 2) {
            type_offset = base_type_uint8_offset + 20;
        } else if (v->size == 4) {
            type_offset = base_type_uint8_offset + 30;
        } else {
            type_offset = base_type_uint8_offset + 40;  /* byte array */
        }
        write_u32(f, type_offset);
        
        /* DW_AT_location (data4) - address offset in section */
        write_u32(f, v->address);
    }
    
    write_u8(f, 0);                  /* DIE terminator */
    
    /* Fill in unit length */
    long end = ftell(f);
    fseek(f, unit_len_pos, SEEK_SET);
    write_u32(f, end - unit_len_pos - 4);
    fseek(f, end, SEEK_SET);
    
    *size = end - start;
}

/**
 * Write DWARF4 Line Number Table (.debug_line)
 */
static void elf_write_debug_line(FILE *f, uint32_t text_addr, long *size) {
    long start = ftell(f);
    
    /* Prologue */
    long total_len_pos = ftell(f);
    write_u32(f, 0);                 /* unit_length (filled later) */
    
    write_u16(f, 4);                 /* version = 4 */
    
    long header_len_pos = ftell(f);
    write_u32(f, 0);                 /* header_length (filled later) */
    
    write_u8(f, 1);                  /* minimum_instruction_length */
    write_u8(f, 1);                  /* maximum_operations_per_instruction */
    write_u8(f, 1);                  /* default_is_stmt */
    write_u8(f, -4 & 0xff);          /* line_base */
    write_u8(f, 2);                  /* line_range */
    write_u8(f, 10);                 /* opcode_base */
    
    /* Standard opcode lengths */
    for (int i = 1; i < 10; i++) {
        write_u8(f, 0);  /* lengths for opcodes 1-9 */
    }
    
    /* Include directories (empty) */
    write_u8(f, 0);                  /* terminator */
    
    /* File names */
    file_info_t *file = _debug_files;
    while (file != NULL) {
        fputs(file->name, f);
        fputc('\0', f);
        write_u8(f, 0);              /* directory index */
        write_u8(f, 0);              /* modification time */
        write_u8(f, 0);              /* file size */
        file = file->next;
    }
    write_u8(f, 0);                  /* file names terminator */
    
    long header_end = ftell(f);
    
    /* Fill header length */
    fseek(f, header_len_pos, SEEK_SET);
    write_u32(f, header_end - header_len_pos - 4);
    fseek(f, header_end, SEEK_SET);
    
    /* Line program: state machine */
    uint32_t current_addr = text_addr;
    uint32_t current_line = 1;
    uint32_t current_file = 1;
    
    line_info_t *line = _debug_lines;
    while (line != NULL) {
        uint32_t addr = line->address + text_addr;
        uint16_t line_num = line->line;
        uint16_t col = line->column;
        uint16_t file_idx = line->file_idx;
        
        /* Skip duplicates */
        if (line->next != NULL && line->address == line->next->address) {
            line = line->next;
            continue;
        }
        
        /* Set address if different */
        if (addr != current_addr) {
            write_u8(f, DW_LNS_extended_op);
            write_uleb128(f, 5);     /* operand length */
            write_u8(f, DW_LNE_set_address);
            write_u32(f, addr);
            current_addr = addr;
        }
        
        /* Advance line if different */
        if (line_num != current_line) {
            write_u8(f, DW_LNS_advance_line);
            write_sleb128(f, (int32_t)line_num - (int32_t)current_line);
            current_line = line_num;
        }
        
        /* Copy current line */
        write_u8(f, DW_LNS_copy);
        
        line = line->next;
    }
    
    /* End sequence */
    write_u8(f, DW_LNS_extended_op);
    write_uleb128(f, 1);
    write_u8(f, DW_LNE_end_sequence);
    
    /* Fill unit length */
    long end = ftell(f);
    fseek(f, total_len_pos, SEEK_SET);
    write_u32(f, end - total_len_pos - 4);
    fseek(f, end, SEEK_SET);
    
    *size = end - start;
}

/**
 * Main ELF32+DWARF4 generation
 */
void elf_write(const char *filename, uint32_t text_addr, const uint8_t *binary, 
               uint32_t binary_size) {
    if (_verbose) {
        fprintf(stderr, "[DEBUG] elf_write: Starting ELF generation...\n");
        fprintf(stderr, "[DEBUG] filename=%s, text_addr=0x%x, binary_size=%u\n",
                filename, text_addr, binary_size);
    }

    /* Gather section info from linker's section state */
    section_t *text_sec  = section_find("text");
    section_t *data_sec  = section_find("data");
    section_t *bss_sec   = section_find("bss");

    uint32_t text_start  = text_sec  ? (uint32_t)text_sec->start_pos  : text_addr;
    uint32_t text_size   = text_sec  ? (uint32_t)text_sec->size       : binary_size;
    uint32_t data_start  = data_sec  ? (uint32_t)data_sec->start_pos  : text_start + text_size;
    uint32_t data_size   = data_sec  ? (uint32_t)data_sec->size       : 0;
    uint32_t bss_start   = bss_sec   ? (uint32_t)bss_sec->start_pos   : data_start + data_size;
    uint32_t bss_size    = bss_sec   ? (uint32_t)bss_sec->size        : 0;

    /* Binary layout: text | [pad] | data - bss is zero-init, not in file */
    /* File binary_size covers text + data (and any padding between them) */

    if (_verbose) {
        fprintf(stderr, "[DEBUG] elf_write: text 0x%x+%u, data 0x%x+%u, bss 0x%x+%u\n",
                text_start, text_size, data_start, data_size, bss_start, bss_size);
    }
    
    FILE *f = fopen(filename, "wb");
    if (f == NULL) {
        perror(filename);
        return;
    }
    
    if (_verbose) {
        fprintf(stderr, "[DEBUG] elf_write: File opened successfully\n");
    }
    
    /* Collect variables from data/bss sections */
    if (_verbose) {
        fprintf(stderr, "[DEBUG] elf_write: Collecting variables...\n");
    }
    collect_variables();
    if (_verbose) {
        fprintf(stderr, "[DEBUG] elf_write: Found %d variables/labels\n", var_count);
    }
    
    /* Collect all strings first */
    strtab_count = 0;
    strtab_entries = NULL;
    collect_debug_strings();
    
    if (_verbose) {
        fprintf(stderr, "[DEBUG] elf_write: Collected %d strings\n", strtab_count);
    }

    /* Determine number of PT_LOAD segments (1=text-only, 2=text+data) */
    int num_phdrs = (data_size > 0) ? 2 : 1;
    /* 10 or 11 section headers: NULL,.text[,.data[,.bss]],.symtab,.strtab,
       .debug_abbrev,.debug_info,.debug_line,.debug_str,.shstrtab */
    int num_sections = 6 + (data_size > 0 ? 1 : 0) + (bss_size > 0 ? 1 : 0) + 3;
    /* Simplified: always use 9 sections for now (no separate .data/.bss SHDRs yet) */
    num_sections = 9;

    /* Calculate offsets */
    uint32_t elf_hdr_size  = 52;
    uint32_t phdr_size     = 32;
    uint32_t ph_offset     = elf_hdr_size;
    uint32_t text_offset   = ph_offset + num_phdrs * phdr_size;
    uint32_t symtab_offset = ((text_offset + binary_size + 15) / 16) * 16;
    
    uint32_t strtab_offset     = symtab_offset + 768;
    uint32_t debug_abbrev_offset = strtab_offset + 777;
    uint32_t debug_info_offset   = debug_abbrev_offset + 512;
    uint32_t debug_line_offset   = debug_info_offset + 2048;
    uint32_t debug_str_offset    = debug_line_offset + 1024;
    uint32_t shstrtab_offset     = debug_str_offset + 2048;
    uint32_t sh_offset           = ((shstrtab_offset + 128 + 3) / 4) * 4;
    
    if (_verbose) {
        fprintf(stderr, "[DEBUG] elf_write: Writing headers...\n");
    }
    
    /* Write ELF header */
    elf_write_header(f, text_start, ph_offset, sh_offset, num_sections, 8 + (data_size > 0 ? 1 : 0) + (bss_size > 0 ? 1 : 0));

    /* Write program header(s) */
    /* PT_LOAD for text (R+X) */
    elf_write_program_header(f, text_start, text_size);

    /* PT_LOAD for data (R+W) if present */
    if (data_size > 0) {
        uint32_t data_file_offset = text_offset + (data_start - text_start);
        /* Write PT_LOAD manually for data with proper flags */
        write_u32(f, PT_LOAD);                          /* p_type */
        write_u32(f, data_file_offset);                 /* p_offset */
        write_u32(f, data_start);                       /* p_vaddr */
        write_u32(f, data_start);                       /* p_paddr */
        write_u32(f, data_size);                        /* p_filesz */
        write_u32(f, data_size + bss_size);             /* p_memsz (includes bss) */
        write_u32(f, PF_R | PF_W);                      /* p_flags */
        write_u32(f, 0x1000);                           /* p_align */
    }
    
    if (_verbose) {
        fprintf(stderr, "[DEBUG] elf_write: Writing binary section (%u bytes)...\n", binary_size);
    }
    
    /* Write binary (text + data) */
    if (binary != NULL) {
        fwrite(binary, 1, binary_size, f);
    } else {
        for (uint32_t i = 0; i < binary_size; i++) {
            fputc(0, f);
        }
    }
    
    /* Pad to next alignment */
    while (ftell(f) < (long)symtab_offset) {
        fputc(0, f);
    }
    
    if (_verbose) {
        fprintf(stderr, "[DEBUG] elf_write: Writing symtab...\n");
    }
    
    /* Write .symtab section */
    for (int i = 0; i < 48; i++) {
        write_u32(f, 0);  /* st_name */
        write_u32(f, 0);  /* st_value */
        write_u32(f, 0);  /* st_size */
        write_u8(f, 0);   /* st_info */
        write_u8(f, 0);   /* st_other */
        write_u16(f, 0);  /* st_shndx */
    }
    
    if (_verbose) {
        fprintf(stderr, "[DEBUG] elf_write: Writing strtab...\n");
    }
    
    /* Write .strtab section */
    strtab_finalize(f);
    
    /* Pad */
    while (ftell(f) < (long)debug_abbrev_offset) {
        fputc(0, f);
    }
    
    if (_verbose) {
        fprintf(stderr, "[DEBUG] elf_write: Writing debug_abbrev...\n");
    }
    
    /* Write .debug_abbrev section */
    long debug_abbrev_size;
    elf_write_debug_abbrev(f, &debug_abbrev_size);
    
    /* Pad */
    while (ftell(f) < (long)debug_info_offset) {
        fputc(0, f);
    }
    
    if (_verbose) {
        fprintf(stderr, "[DEBUG] elf_write: Writing debug_info...\n");
    }
    
    /* Write .debug_info section */
    long debug_info_size;
    elf_write_debug_info(f, text_start, text_size, debug_abbrev_offset, 
                         debug_str_offset, &debug_info_size);
    
    /* Pad */
    while (ftell(f) < (long)debug_line_offset) {
        fputc(0, f);
    }
    
    if (_verbose) {
        fprintf(stderr, "[DEBUG] elf_write: Writing debug_line...\n");
    }
    
    /* Write .debug_line section */
    long debug_line_size;
    elf_write_debug_line(f, text_start, &debug_line_size);
    
    /* Pad */
    while (ftell(f) < (long)debug_str_offset) {
        fputc(0, f);
    }
    
    if (_verbose) {
        fprintf(stderr, "[DEBUG] elf_write: Writing debug_str...\n");
    }
    
    /* Write .debug_str section */
    fputs("HC SDK Retro", f);
    for (int i = 0; i < 2000; i++) fputc(0, f);
    
    /* Pad */
    while (ftell(f) < (long)shstrtab_offset) {
        fputc(0, f);
    }
    
    if (_verbose) {
        fprintf(stderr, "[DEBUG] elf_write: Writing shstrtab...\n");
    }
    
    /* Write .shstrtab section */
    const char *section_names[] = {
        "", ".text", ".symtab", ".strtab", ".debug_abbrev", 
        ".debug_info", ".debug_line", ".debug_str", ".shstrtab"
    };
    for (int i = 0; i < 9; i++) {
        fputs(section_names[i], f);
        fputc(0, f);
    }
    
    /* Pad to section headers */
    while (ftell(f) < (long)sh_offset) {
        fputc(0, f);
    }
    
    if (_verbose) {
        fprintf(stderr, "[DEBUG] elf_write: Writing section headers...\n");
    }
    
    /* Write section headers */
    struct {
        uint32_t name;
        uint32_t type;
        uint32_t flags;
        uint32_t addr;
        uint32_t offset;
        uint32_t size;
        uint32_t link;
        uint32_t info;
        uint32_t addralign;
        uint32_t entsize;
    } sections[9];
    
    /* Initialize sections using memset and individual assignments */
    memset(sections, 0, sizeof(sections));
    
    /* Section 0: NULL */
    sections[0].name = 0;
    sections[0].type = SHT_NULL;
    
    /* Section 1: .text */
    sections[1].name = 1;
    sections[1].type = SHT_PROGBITS;
    sections[1].flags = SHF_ALLOC | SHF_EXECINSTR;
    sections[1].addr = text_start;
    sections[1].offset = text_offset;
    sections[1].size = text_size;
    sections[1].addralign = 1;
    
    /* Section 2: .symtab */
    sections[2].name = 7;
    sections[2].type = SHT_SYMTAB;
    sections[2].offset = symtab_offset;
    sections[2].size = 768;
    sections[2].link = 3;
    sections[2].addralign = 4;
    sections[2].entsize = 16;
    
    /* Section 3: .strtab */
    sections[3].name = 15;
    sections[3].type = SHT_STRTAB;
    sections[3].offset = strtab_offset;
    sections[3].size = 777;
    sections[3].addralign = 1;
    
    /* Section 4: .debug_abbrev */
    sections[4].name = 23;
    sections[4].type = SHT_PROGBITS;
    sections[4].offset = debug_abbrev_offset;
    sections[4].size = debug_abbrev_size;
    sections[4].addralign = 1;
    
    /* Section 5: .debug_info */
    sections[5].name = 37;
    sections[5].type = SHT_PROGBITS;
    sections[5].offset = debug_info_offset;
    sections[5].size = debug_info_size;
    sections[5].addralign = 1;
    
    /* Section 6: .debug_line */
    sections[6].name = 49;
    sections[6].type = SHT_PROGBITS;
    sections[6].offset = debug_line_offset;
    sections[6].size = debug_line_size;
    sections[6].addralign = 1;
    
    /* Section 7: .debug_str */
    sections[7].name = 61;
    sections[7].type = SHT_PROGBITS;
    sections[7].offset = debug_str_offset;
    sections[7].size = 2048;
    sections[7].addralign = 1;
    
    /* Section 8: .shstrtab */
    sections[8].name = 72;
    sections[8].type = SHT_STRTAB;
    sections[8].offset = shstrtab_offset;
    sections[8].size = 256;
    sections[8].addralign = 1;
    
    for (int i = 0; i < 9; i++) {
        write_u32(f, sections[i].name);
        write_u32(f, sections[i].type);
        write_u32(f, sections[i].flags);
        write_u32(f, sections[i].addr);
        write_u32(f, sections[i].offset);
        write_u32(f, sections[i].size);
        write_u32(f, sections[i].link);
        write_u32(f, sections[i].info);
        write_u32(f, sections[i].addralign);
        write_u32(f, sections[i].entsize);
    }
    
    long final_size = ftell(f);
    fclose(f);
    
    /* Cleanup */
    free(strtab_entries);
    strtab_entries = NULL;
    strtab_count = 0;
    
    free(var_list);
    var_list = NULL;
    var_count = 0;
    
    if (_verbose) {
        fprintf(stderr, "[DEBUG] elf_write: Complete! File size: %ld bytes\n", final_size);
        fprintf(stderr, "ELF32+DWARF4 debug info written to: %s\n", filename);
    }
}
