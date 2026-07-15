#include "link.h"

// [English] Debug info: maintains list of source files and line-to-address mappings
// [Portuguese] Info de debug: mantém lista de arquivos fonte e mapeamentos linha-para-endereço

// [English] Find or create file entry in debug file list
// [Portuguese] Encontra ou cria entrada de arquivo na lista de arquivos debug
static uint16_t _file_count = 0;

int debug_add_file(char *filename)
{
    if (!filename)
        return 0;

    // [English] Check if file already exists
    // [Portuguese] Verifica se arquivo já existe
    file_info_t *f = _debug_files;
    uint16_t idx = 0;
    while (f)
    {
        idx++;
        if (!strcmp(f->name, filename))
            return idx;
        f = f->next;
    }

    // [English] Add new file entry
    // [Portuguese] Adiciona nova entrada de arquivo
    size_t len = strlen(filename);
    file_info_t *new_file = malloc(sizeof(file_info_t) + len);
    if (!new_file)
    {
        error("out of memory");
    }
    strcpy(new_file->name, filename);
    new_file->next = _debug_files;
    _debug_files = new_file;
    _file_count++;
    return _file_count;
}

void debug_add_line(uint32_t address, uint16_t file_idx, uint16_t line, uint16_t column)
{
    if (file_idx == 0 || line == 0)
        return;

    // [English] Create new line mapping entry
    // [Portuguese] Cria nova entrada de mapeamento de linha
    line_info_t *new_line = malloc(sizeof(line_info_t));
    if (!new_line)
    {
        error("out of memory");
    }
    new_line->address = address;
    new_line->file_idx = file_idx;
    new_line->line = line;
    new_line->column = column;
    new_line->next = _debug_lines;
    _debug_lines = new_line;
}

void debug_print(char *dbg_name)
{
    if (!dbg_name)
        return;

    // [English] Open debug output file
    // [Portuguese] Abre arquivo de saída de debug
    FILE *dbg = fopen(dbg_name, "w");
    if (!dbg)
    {
        error("can't open debug file: %s", dbg_name);
    }

    // [English] Write file list section
    // [Portuguese] Escreve seção de lista de arquivos
    fprintf(dbg, "; Debug info file - HC SDK Retro v%d.%d R%d\n", VERSION, SUBVERSION, REVISION);
    fprintf(dbg, "; Format: simple text mappings for debugger integration\n\n");
    fprintf(dbg, "[FILES]\n");
    
    file_info_t *f = _debug_files;
    uint16_t file_idx = 1;
    while (f)
    {
        fprintf(dbg, "%d: %s\n", file_idx, f->name);
        f = f->next;
        file_idx++;
    }
    
    // [English] Write line mapping section
    // [Portuguese] Escreve seção de mapeamento de linhas
    fprintf(dbg, "\n[LINES]\n");
    fprintf(dbg, "; format: ADDRESS FILEIDX:LINE:COLUMN\n");
    
    // [English] Sort lines by address for better readability
    // [Portuguese] Ordena linhas por endereço para melhor legibilidade
    line_info_t *sorted_lines = NULL;
    line_info_t *current = _debug_lines;
    
    while (current)
    {
        line_info_t *next = current->next;
        
        // [English] Insert in sorted order
        // [Portuguese] Insere em ordem classificada
        if (!sorted_lines || current->address < sorted_lines->address)
        {
            current->next = sorted_lines;
            sorted_lines = current;
        }
        else
        {
            line_info_t *pos = sorted_lines;
            while (pos->next && pos->next->address < current->address)
                pos = pos->next;
            current->next = pos->next;
            pos->next = current;
        }
        current = next;
    }
    
    /* Update global pointer to sorted list for elf_write */
    _debug_lines = sorted_lines;
    
    // [English] Write sorted line mappings (skip duplicates)
    // [Portuguese] Escreve mapeamentos de linha classificados (pula duplicatas)
    line_info_t *line = sorted_lines;
    uint32_t last_address = (uint32_t)-1;
    uint16_t last_file_idx = 0;
    uint16_t last_line = 0;
    while (line)
    {
        // [English] Skip duplicate entries (same address and line)
        // [Portuguese] Pula entradas duplicadas (mesmo endereço e linha)
        if (line->address != last_address || line->file_idx != last_file_idx || line->line != last_line)
        {
            fprintf(dbg, "0x%04X %d:%d:%d\n", line->address, line->file_idx, line->line, line->column);
            last_address = line->address;
            last_file_idx = line->file_idx;
            last_line = line->line;
        }
        line = line->next;
    }
    
    // [English] Write symbols section from existing symbol data
    // [Portuguese] Escreve seção de símbolos a partir de dados de símbolo existentes
    fprintf(dbg, "\n[SYMBOLS]\n");
    fprintf(dbg, "; format: ADDRESS NAME [TYPE SCOPE]\n");
    
    const_t *c = _consts;
    while (c)
    {
        if (c->obj && c->obj->use_in_link)
        {
            fprintf(dbg, "0x%04X %s [%s %s]\n", 
                    (uint16_t)c->value, 
                    c->name,
                    c->is_offset ? "LABEL" : "CONST",
                    c->is_global ? "GLOBAL" : "LOCAL");
        }
        c = c->next;
    }
    
    fclose(dbg);
}
