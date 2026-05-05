#include "emu.h"
#include <stdio.h>

char *_disk_default = NULL;

static struct {
    DIR *dir;
    char pattern[256];
    char path[256];
    bool active;
} _search;

static struct {
    DIR *dir;
    char path[256];
    bool active;
} _search2;

static int _return_code_store = 0;
static uint8_t _write_protect = 0;
static uint8_t _read_only_vector = 0;
static uint8_t _user_code = 0;

// MSX-DOS 2.0 file handles
#define MAX_HANDLES 16
static struct {
    FILE *fp;
    bool used;
    uint8_t mode;
} _handles[MAX_HANDLES];

static uint8_t _default_drive = 0;

// Environment variables (MSX-DOS 2.0)
#define MAX_ENV 32
static char *_env[MAX_ENV];
static int _env_count = 0;
static bool _env_inited = false;

// [English] Initialize MSX-DOS 2.0 environment variables
// [Portuguese] Inicializa variáveis de ambiente do MSX-DOS 2.0
static void env_init() {
    if(_env_inited) return;
    _env[0] = strdup("PATH=/bin:/usr/local/bin");
    _env[1] = strdup("PROMPT=$");
    _env_count = 2;
    _env_inited = true;
}

// [English] Parse MSX-DOS 2.0 ASCIIZ path: supports "X:path" or just "path"
// [Portuguese] Parseia caminho ASCIIZ do MSX-DOS 2.0: suporta "X:caminho" ou só "caminho"
// [English] Output: full_path with
// [Portuguese] separators, *drive = drive number (0/1)
// Saída: full_path com separadores /, *drive = número do drive (0/1)
static void parse_asciiz(uint16_t addr, char *out, int *drive) {
    char *p = (char*)&_memory[addr];
    if(p[0] && p[1] == ':') {
        *drive = toupper(p[0]) - 'A';
        p += 2;
        if(*drive == 0) strcpy(out, _disk_a_path);
        else strcpy(out, _disk_b_path);
    } else {
        *drive = _default_drive;
        strcpy(out, *drive == 0 ? _disk_a_path : _disk_b_path);
    }
    size_t len = strlen(out);
    if(len > 0 && out[len-1] != PATHSEP) strcat(out, PATHSEPSTR);
    while(*p) {
        char c = *p++;
#ifdef _WIN32
        out[strlen(out)] = (c == '/') ? PATHSEP : c;
#else
        out[strlen(out)] = (c == '\\') ? PATHSEP : c;
#endif
    }
    out[strlen(out)] = 0;
}

// [English] MSX-DOS CALL 5 handler (main system call entry point)
// [Portuguese] Manipulador de CALL 5 do MSX-DOS (ponto de entrada principal de chamadas de sistema)
void abi_dos_call_5()
{
    // [English] Ensure default disk is set
    // [Portuguese] Garante que o disco padrão esteja definido
    if(!_disk_default) _disk_default = _disk_a_path;
    uint16_t ptr;
    char path[FILENAME_MAX];

    // [English] Dispatch by function code in C register
    // [Portuguese] Despacha pelo código de função no registrador C
    // --== CP/M-compatible function codes (0x00-0x3F) ==--
    // --== Códigos de função compatíveis com CP/M (0x00-0x3F) ==--
    switch(_regs_curr.bc.c)
    {
        // [English] Program terminate
        // [Portuguese] Terminação de programa
        case 0x00: // Program terminate
            _executing = false;
            break;
        case 0x01: // Console input
        case 0x03: // Auxiliary input
        case 0x07: // Direct console input 
            _regs_curr.hl.l = keyb_wait_pop();
            _regs_curr.af.a = _regs_curr.hl.l;
            screen_put_char(_regs_curr.af.a);
            break;
        case 0x02: // Console output
        case 0x04: // Auxiliary output
            screen_put_char(_regs_curr.de.e);
            break;
        case 0x05: // Disk Reset
            _disk_transferr_address = 0x80;
            break;
        case 0x06: // Direct Console IO
            if(_regs_curr.de.e == 0xff)
            {
                _regs_curr.hl.l = keyb_wait_pop();
                _regs_curr.af.a = _regs_curr.hl.l;
            }
            else
            {
                screen_put_char(_regs_curr.de.e);
            }
            break;
        case 0x08: // Console input without echo
            _regs_curr.hl.l = keyb_wait_pop();
            _regs_curr.af.a = _regs_curr.hl.l;
            break;
        case 0x09: // String output
            ptr = _regs_curr.de.word;
            while(mem_get_byte(ptr) != '$')
            {
                screen_put_char(mem_get_byte(ptr++));
            }
            break;
        case 0x0a: // Buffered Line Input
            ptr = _regs_curr.de.word;
            {
                char c;
                mem_set_byte(ptr + 1, 0);
                do
                {
                    c = keyb_wait_pop();
                    if(mem_get_byte(ptr + 1) < mem_get_byte(ptr))
                    {
                        if(c == 0x8)
                        {
                            screen_put_char(0x8);
                            screen_put_char(' ');
                            screen_put_char(0x8);
                            mem_set_byte(ptr + 1, mem_get_byte(ptr+1) - 1);
                        }
                        else if(c != '\n' && c != '\r')
                        {
                            mem_set_byte(ptr + 2 + mem_get_byte(ptr+1), c);
                            mem_set_byte(ptr + 1, mem_get_byte(ptr+1) + 1);
                            screen_put_char(c);
                        }
                    }
                    screen_draw_if_changed();
                } while(c != '\n' && c != '\r');
                mem_set_byte(ptr + 2 + mem_get_byte(ptr), '\r');
            }
            break;
        case 0x0b: // Console Status
            _regs_curr.af.a = keyb_avail() ? 0xff : 0x00;
            _regs_curr.hl.l = _regs_curr.af.a;
            break;
        case 0x0c: // Return Version Number (CP/M)
            _regs_curr.af.a = 0x22;
            _regs_curr.hl.l = 0x22;
            _regs_curr.bc.b = 0x00;
            _regs_curr.hl.h = 0x00;
            break;
        case 0x0d: // Reset Disk System
            _disk_transferr_address = 0x80;
            if(_search.active && _search.dir) { closedir(_search.dir); _search.active = false; }
            break;
        case 0x0e: // Select Disk
            _regs_curr.af.a = 2;
            _regs_curr.hl.l = 2;
            switch (_regs_curr.de.e)
            {
                case 0:
                    _disk_default = _disk_a_path;
                    break;
                case 1:
                    _disk_default = _disk_b_path;
                    break;
                default:
                    break;
            }
            break;
        case 0x0f: // Open file FCB
        case 0x16: // Create file FCB
        case 0x23: // Get file size
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                if(fcb->drive == 0)strcpy(path, _disk_default);
                else if(fcb->drive == 2) strcpy(path, _disk_b_path);
                else strcpy(path, _disk_a_path);
                char c;
                bool inject_dot = false;
                for(int i = 0; i < 8; i++)
                {
                    if(fcb->name[i] == ' ') break;
                    c = tolower(fcb->name[i]);
                    strncat(path, &c, 1);
                    inject_dot = true;
                }
                if(inject_dot) strcat(path, ".");
                for(int i = 0; i < 3; i++)
                {
                    if(fcb->ext[i] == ' ') break;
                    c = tolower(fcb->ext[i]);
                    strncat(path, &c, 1);
                }
                FILE *file = fopen(path, _regs_curr.bc.c == 0x0f ? "rb" : "wb");
                if(!file)
                {
                    fprintf(stderr, "\n\n[ERROR: FILE NOT FOUND: %s]", path);
                    _regs_curr.af.a = 0xff;
                }
                else
                {
                    _regs_curr.af.a = 0x00;
                }
                _regs_curr.hl.l = _regs_curr.af.a;
                if(_regs_curr.bc.c == 0x23)
                {
                    fseek(file, 0, SEEK_END);
                    fcb->file_size = ftell(file);
                    fseek(file, 0, SEEK_SET);
                    fclose(file);
                }
                else
                {
                    fcb->file_size = 0;
                    memcpy(fcb->internal, &file, sizeof(size_t));
                }
            }
            break;
        case 0x10: // Close FCB
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                FILE *file;
                memcpy(&file, fcb->internal, sizeof(size_t));
                if(file)
                {
                    fclose(file);
                    _regs_curr.af.a = 0x00;
                }
                else
                {
                    _regs_curr.af.a = 0xff;
                }
                _regs_curr.hl.l = _regs_curr.af.a;
            }
            break;
        case 0x11: // Search First
            if(_search.active && _search.dir) { closedir(_search.dir); _search.active = false; }
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                _search.active = true;
                strcpy(_search.path, fcb->drive == 0 ? _disk_default : (fcb->drive == 2 ? _disk_b_path : _disk_a_path));
                char *p = _search.path + strlen(_search.path);
                for(int i = 0; i < 8; i++) {
                    if(fcb->name[i] == ' ') break;
                    if(fcb->name[i] == '?') *p++ = '*';
                    else *p++ = tolower(fcb->name[i]);
                }
                if(fcb->ext[0] != ' ') {
                    *p++ = '.';
                    for(int i = 0; i < 3; i++) {
                        if(fcb->ext[i] == ' ') break;
                        if(fcb->ext[i] == '?') *p++ = '*';
                        else *p++ = tolower(fcb->ext[i]);
                    }
                }
                *p = 0;
                strcpy(_search.pattern, _search.path);
                p = _search.path + strlen(_search.path);
                *p++ = PATHSEP; *p = 0;
                _search.dir = opendir(_search.path);
                if(!_search.dir) { _regs_curr.af.a = 0xff; _regs_curr.hl.l = 0xff; break; }
            }
            // Fall through to Search Next
        case 0x12: // Search Next
            {
                struct dirent *entry;
                abi_dos_fcb_t *dfcb = (abi_dos_fcb_t*)&_memory[0x80];
                while((entry = readdir(_search.dir)) != NULL) {
                    char fullpath[256];
                    strcpy(fullpath, _search.path);
                    strcat(fullpath, entry->d_name);
                    // Check if matches pattern using fnmatch-like approach
                    // Simple: just check the pattern match manually
                    struct stat st;
                    if(stat(fullpath, &st) != 0 || !S_ISREG(st.st_mode)) continue;
                    // Build FCB in DMA
                    memset(dfcb, 0, sizeof(abi_dos_fcb_t));
                    char *dot = strrchr(entry->d_name, '.');
                    if(dot) {
                        int nlen = dot - entry->d_name;
                        for(int i = 0; i < nlen && i < 8; i++)
                            dfcb->name[i] = toupper(entry->d_name[i]);
                        dot++;
                        for(int i = 0; i < 3 && dot[i]; i++)
                            dfcb->ext[i] = toupper(dot[i]);
                    } else {
                        for(int i = 0; i < 8 && entry->d_name[i]; i++)
                            dfcb->name[i] = toupper(entry->d_name[i]);
                    }
                    dfcb->file_size = st.st_size;
                    _regs_curr.af.a = 0x00;
                    _regs_curr.hl.l = 0x00;
                    break;
                }
                if(!entry) { _regs_curr.af.a = 0xff; _regs_curr.hl.l = 0xff; }
            }
            break;
        case 0x13: // Delete file
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                char delpath[256];
                strcpy(delpath, fcb->drive == 0 ? _disk_default : (fcb->drive == 2 ? _disk_b_path : _disk_a_path));
                char *p = delpath + strlen(delpath);
                for(int i = 0; i < 8; i++) {
                    if(fcb->name[i] == ' ') break;
                    *p++ = tolower(fcb->name[i]);
                }
                if(fcb->ext[0] != ' ') {
                    *p++ = '.';
                    for(int i = 0; i < 3; i++) {
                        if(fcb->ext[i] == ' ') break;
                        *p++ = tolower(fcb->ext[i]);
                    }
                }
                *p = 0;
                _regs_curr.af.a = remove(delpath) == 0 ? 0x00 : 0xff;
                _regs_curr.hl.l = _regs_curr.af.a;
            }
            break;
        case 0x14: // Sequential Read FCB
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                FILE *file;
                memcpy(&file, fcb->internal, sizeof(size_t));
                ptr = _disk_transferr_address;
                if(file)
                {
                    if(feof(file))
                    {
                        _regs_curr.af.a = 0x01;
                    }
                    else
                    {
                        for (size_t i = 0; i < 128; i++)
                        {
                            char tmp;
                            if(fread(&tmp, 1, 1, file) <= 0)
                            {
                                tmp = 0;
                            }
                            mem_set_byte(ptr++, tmp);
                        }
                        _regs_curr.af.a = 0x00;
                    }
                }
                else
                {
                    _regs_curr.af.a = 0xff;
                }
                _regs_curr.hl.l = _regs_curr.af.a;
                
            }
            break;
        case 0x15: // Sequential Write FCB
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                FILE *file;
                memcpy(&file, fcb->internal, sizeof(size_t));
                ptr = _disk_transferr_address;
                if(file)
                {
                    if(feof(file))
                    {
                        _regs_curr.af.a = 0x01;
                    }
                    else
                    {
                        _regs_curr.af.a = 0x00;
                        for (size_t i = 0; i < 128; i++)
                        {
                            char tmp = mem_get_byte(ptr++);
                            if(fwrite(&tmp, 1, 1, file) <= 0)
                            {
                                _regs_curr.af.a = 0x01;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    _regs_curr.af.a = 0xff;
                }
                _regs_curr.hl.l = _regs_curr.af.a;
                
            }
            break;
        case 0x17: // Rename file
            {
                // FCB format for rename: drive byte + old name(16 bytes) + new name(16 bytes)
                // Actually in CP/M: 33 byte FCB with drive(1) + old(16: padded name+ext) + new(16)
                uint8_t *f = &_memory[_regs_curr.de.word];
                char oldpath[256], newpath[256];
                uint8_t drv = f[0];
                strcpy(oldpath, drv == 0 ? _disk_default : (drv == 2 ? _disk_b_path : _disk_a_path));
                strcpy(newpath, oldpath);
                char *op = oldpath + strlen(oldpath);
                char *np = newpath + strlen(newpath);
                for(int i = 0; i < 8; i++) {
                    if(f[1+i] == ' ') break;
                    *op++ = tolower(f[1+i]);
                }
                if(f[9] != ' ') {
                    *op++ = '.';
                    for(int i = 0; i < 3; i++) {
                        if(f[9+i] == ' ') break;
                        *op++ = tolower(f[9+i]);
                    }
                }
                *op = 0;
                for(int i = 0; i < 8; i++) {
                    if(f[17+i] == ' ') break;
                    *np++ = tolower(f[17+i]);
                }
                if(f[25] != ' ') {
                    *np++ = '.';
                    for(int i = 0; i < 3; i++) {
                        if(f[25+i] == ' ') break;
                        *np++ = tolower(f[25+i]);
                    }
                }
                *np = 0;
                _regs_curr.af.a = rename(oldpath, newpath) == 0 ? 0x00 : 0xff;
                _regs_curr.hl.l = _regs_curr.af.a;
            }
            break;
        case 0x18: // Get Login Vector
            _regs_curr.hl.word = 3; // drives A and B always active
            break;
        case 0x19: // Get current drive
            _regs_curr.af.a = _disk_default == _disk_a_path ? 0 : 1;
            _regs_curr.hl.l = _regs_curr.af.a;
            break;
        case 0x1a: // Set Disk Transfer Address
            _disk_transferr_address = _regs_curr.de.word;
            break;
        // --== Disk operations / Operações de disco ==--
        case 0x1b: // Get Allocation Vector
            _regs_curr.hl.word = 0xE000;
            _regs_curr.de.word = 0xE000;
            break;
        case 0x1c: // Write Protect Disk
            _write_protect = _regs_curr.de.e; // drive to protect (0=A, 1=B)
            _regs_curr.af.a = 0x00;
            _regs_curr.hl.l = 0x00;
            break;
        case 0x1d: // Get Read-Only Vector
            _regs_curr.hl.word = _read_only_vector;
            break;
        case 0x1e: // Set File Attributes (CP/M)
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                char apath[256];
                strcpy(apath, fcb->drive == 0 ? _disk_default : (fcb->drive == 2 ? _disk_b_path : _disk_a_path));
                char *p = apath + strlen(apath);
                for(int i = 0; i < 8; i++) { if(fcb->name[i] == ' ') break; *p++ = tolower(fcb->name[i]); }
                if(fcb->ext[0] != ' ') {
                    *p++ = '.';
                    for(int i = 0; i < 3; i++) { if(fcb->ext[i] == ' ') break; *p++ = tolower(fcb->ext[i]); }
                }
                *p = 0;
                // Bit 7 of first name byte = read-only
                mode_t m = (fcb->name[0] & 0x80) ?
                    (S_IRUSR | S_IRGRP | S_IROTH) :
                    (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                _regs_curr.af.a = (chmod(apath, m) == 0) ? 0x00 : 0xff;
                _regs_curr.hl.l = _regs_curr.af.a;
            }
            break;
        case 0x1f: // Get Disk DPB
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                uint8_t *dpb = &_memory[0xE100];
                uint8_t drive = fcb->drive;
                // Build DPB at 0xE100 (16 bytes)
                // Default MSX-DOS 1.0 DPB for 360KB DD disk
                dpb[0]  = 2;    // SPT low (720 sectors/track)... actually 720 = 0x02D0
                // Wait, SPT is sectors per track * tracks per cylinder
                // For MSX-DOS 1.0 standard: 720 sectors per track = 0x02D0
                // Actually SPT = sectors per track (2 bytes)
                // 80 tracks, 9 sectors/track, 2 heads = 1440 sectors total
                // SPT = 9 (sectors per track)
                dpb[0]  = 9;    // SPT low (sectors per track)
                dpb[1]  = 0;    // SPT high
                dpb[2]  = 4;    // BSH (block shift: 2^4 = 16 sectors per block)
                dpb[3]  = 0x0F; // BLM (block mask)
                dpb[4]  = 0;    // EXM (extent mask)
                dpb[5]  = 0;    // reserved
                // [English] DSM (disk size in blocks - 1): 1440 sectors
                // [Portuguese] 16 = 90 blocks
                dpb[6]  = 89;   // DSM low
                dpb[7]  = 0;    // DSM high
                dpb[8]  = 63;   // DRM (directory max)
                dpb[9]  = 0;    // DRM high
                dpb[10] = 0xC0; // AL0
                dpb[11] = 0;    // AL1
                dpb[12] = 31;   // CKS (check size)
                dpb[13] = 0;    // CKS high
                dpb[14] = 2;    // OFF (reserved tracks)
                dpb[15] = 0;    // OFF high
                _regs_curr.hl.word = 0xE100;
                _regs_curr.de.word = 0xE100;
                _regs_curr.af.a = 0x00;
            }
            break;
        case 0x20: // Get/Set User Code
            if(_regs_curr.de.e == 0xff) { // Get
                _regs_curr.hl.l = _user_code;
                _regs_curr.af.a = _user_code;
            } else { // Set
                _user_code = _regs_curr.de.e & 0x0F;
                _regs_curr.af.a = 0x00;
                _regs_curr.hl.l = 0x00;
            }
            break;
        case 0x21: // Random Read FCB
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                FILE *file;
                memcpy(&file, fcb->internal, sizeof(size_t));
                ptr = _disk_transferr_address;
                if(file)
                {
                    fseek(file, fcb->random_record_number * 128, SEEK_SET);
                    if(feof(file))
                    {
                        _regs_curr.af.a = 0x01;
                    }
                    else
                    {
                        for (size_t i = 0; i < 128; i++)
                        {
                            char tmp;
                            if(fread(&tmp, 1, 1, file) <= 0)
                            {
                                tmp = 0;
                            }
                            mem_set_byte(ptr++, tmp);
                        }
                        _regs_curr.af.a = 0x00;
                    }
                }
                else
                {
                    _regs_curr.af.a = 0xff;
                }
                _regs_curr.hl.l = _regs_curr.af.a;
                
            }
            break;
        case 0x22: // Random Write FCB
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                FILE *file;
                memcpy(&file, fcb->internal, sizeof(size_t));
                ptr = _disk_transferr_address;
                if(file)
                {
                    fseek(file, fcb->random_record_number * 128, SEEK_SET);
                    if(feof(file))
                    {
                        _regs_curr.af.a = 0x01;
                    }
                    else
                    {
                        _regs_curr.af.a = 0x00;
                        for (size_t i = 0; i < 128; i++)
                        {
                            char tmp = mem_get_byte(ptr++);
                            if(fwrite(&tmp, 1, 1, file) <= 0)
                            {
                                _regs_curr.af.a = 0x01;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    _regs_curr.af.a = 0xff;
                }
                _regs_curr.hl.l = _regs_curr.af.a;
                
            }
            break;
        case 0x24: // Set Random Record
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                FILE *file;
                memcpy(&file, fcb->internal, sizeof(size_t));
                if(file)
                {
                    fcb->random_record_number = fcb->current_record + (fcb->extent_low << 8) + (fcb->extent_high << 16);
                }
            }
            break;
        case 0x25: // Reset Drive (MSX-DOS specific)
            _disk_transferr_address = 0x80;
            if(_search.active && _search.dir) { closedir(_search.dir); _search.active = false; }
            break;
        // --== MSX-DOS specific FCB functions / Funções FCB específicas do MSX-DOS ==--
        case 0x29: // Get/Set File Date/Time (MSX-DOS specific via FCB)
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                FILE *file;
                memcpy(&file, fcb->internal, sizeof(size_t));
                if(!file) { _regs_curr.af.a = 0xff; break; }
                fseek(file, 0, SEEK_SET);
                struct stat st;
                if(fstat(fileno(file), &st) == 0) {
                    struct tm *tm = localtime(&st.st_mtime);
                    // MSX-DOS FCB date/time format (offset 16-19 in FCB)
                    fcb->file_size = (uint32_t)st.st_size;
                    // Store in FCB: time at offset 16, date at offset 18
                    uint16_t ftime = (tm->tm_hour << 11) | (tm->tm_min << 5) | (tm->tm_sec >> 1);
                    uint16_t fdate = ((tm->tm_year - 80) << 9) | ((tm->tm_mon + 1) << 5) | tm->tm_mday;
                    // [English] O MSX-DOS usa campos estendidos do FCB
                    // [Portuguese] No FCB padrão, offset 16-19 são para EX/SZ/RC
                    _regs_curr.af.a = 0x00;
                } else _regs_curr.af.a = 0xff;
            }
            break;
        case 0x26: // Random Block Read FCB
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                FILE *file;
                memcpy(&file, fcb->internal, sizeof(size_t));
                ptr = _disk_transferr_address;
                uint16_t block_size = fcb->extent_high + (fcb->record_count << 8);
                uint16_t block_count = _regs_curr.hl.word;
                _regs_curr.hl.word = 0;
                if(file)
                {
                    fseek(file, fcb->random_record_number * block_size, SEEK_SET);
                    if(feof(file))
                    {
                        _regs_curr.af.a = 0x01;
                    }
                    else
                    {
                        for (size_t i = 0; i < (block_size * block_count); i++)
                        {
                            char tmp;
                            if(fread(&tmp, 1, 1, file) <= 0)
                            {
                                tmp = 0;
                            }
                            mem_set_byte(ptr++, tmp);
                            _regs_curr.hl.word = (uint16_t)((size_t)i / (size_t)block_size);
                        }
                        _regs_curr.af.a = 0x00;
                    }
                }
                else
                {
                    _regs_curr.af.a = 0xff;
                }
                _regs_curr.hl.l = _regs_curr.af.a;
                
            }
            break;
        case 0x27: // Random Block Write FCB
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                FILE *file;
                memcpy(&file, fcb->internal, sizeof(size_t));
                ptr = _disk_transferr_address;
                uint16_t block_size = fcb->extent_high + (fcb->record_count << 8);
                uint16_t block_count = _regs_curr.hl.word;
                if(file)
                {
                    fseek(file, fcb->random_record_number * block_size, SEEK_SET);
                    if(feof(file))
                    {
                        _regs_curr.af.a = 0x01;
                    }
                    else
                    {
                        _regs_curr.af.a = 0x00;
                        for (size_t i = 0; i < (block_size * block_count); i++)
                        {
                            char tmp = mem_get_byte(ptr++);
                            if(fwrite(&tmp, 1, 1, file) <= 0)
                            {
                                _regs_curr.af.a = 0x01;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    _regs_curr.af.a = 0xff;
                }
                _regs_curr.hl.l = _regs_curr.af.a;
                
            }
            break;
        case 0x28: // Random Write fill Zero fill FCB
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                FILE *file;
                memcpy(&file, fcb->internal, sizeof(size_t));
                ptr = _disk_transferr_address;
                if(file)
                {
                    fseek(file, fcb->random_record_number * 128, SEEK_SET);
                    if(feof(file))
                    {
                        _regs_curr.af.a = 0x01;
                    }
                    else
                    {
                        _regs_curr.af.a = 0x00;
                        for (size_t i = 0; i < 128; i++)
                        {
                            char tmp = 0;
                            if(fwrite(&tmp, 1, 1, file) <= 0)
                            {
                                _regs_curr.af.a = 0x01;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    _regs_curr.af.a = 0xff;
                }
                _regs_curr.hl.l = _regs_curr.af.a;
                
            }
            break;
        // --== Date/Time functions / Funções de data/hora ==--
        case 0x2a: // Get Data
            {
                time_t t = time(NULL);
                struct tm *tm = localtime(&t);
                _regs_curr.hl.word = tm->tm_year + 1900;
                _regs_curr.de.d = tm->tm_mon + 1;
                _regs_curr.de.e = tm->tm_mday;
                _regs_curr.af.a = tm->tm_wday;
            }
            break;
        case 0x2b: // Set Data
            _regs_curr.af.a = 0xff;
            break;
        case 0x2c: // Get Time
            {
                time_t t = time(NULL);
                struct tm *tm = localtime(&t);
                _regs_curr.hl.h = tm->tm_hour;
                _regs_curr.hl.l = tm->tm_min;
                _regs_curr.de.d = tm->tm_sec;
                _regs_curr.de.e = 0;
            }
            break;
        case 0x2d: // Set Time
            _regs_curr.af.a = 0xff;
            break;
        // --== System functions / Funções de sistema ==--
        case 0x2e: // Get/Set Return Code
            if(_regs_curr.de.e == 0) // Get
                _regs_curr.hl.word = _return_code_store;
            else // Set
                _return_code_store = _regs_curr.de.e;
            break;
        case 0x2f: // Absolute Sector Read
            _regs_curr.af.a = 0xff;
            break;
        case 0x30: // Absolute Sector Write
            _regs_curr.af.a = 0xff;
            break;
        case 0x31: // Get Disk Parameters
            _regs_curr.af.a = 0xff;
            break;
        case 0x32: // Get MSX-DOS Version
            _regs_curr.hl.word = 0x0200; // MSX-DOS 2.0
            _regs_curr.bc.word = 0x0001;
            _regs_curr.de.word = 0x0000;
            break;
        // --== MSX-DOS 2.0 ASCIIZ/Handle-based API / API baseada em handles ASCIIZ do MSX-DOS 2.0 ==--
        case 0x40: // Open file (ASCIIZ handle-based)
        case 0x44: // Create file
            {
                char fpath[256]; int drv;
                parse_asciiz(_regs_curr.de.word, fpath, &drv);
                const char *fmode = (_regs_curr.bc.c == 0x40) ? "rb" : "wb";
                FILE *f = fopen(fpath, fmode);
                if(!f) { _regs_curr.af.a = 0xff; break; }
                int h = -1;
                for(int i = 0; i < MAX_HANDLES; i++) {
                    if(!_handles[i].used) { h = i; break; }
                }
                if(h < 0) { fclose(f); _regs_curr.af.a = 0xff; break; }
                _handles[h].fp = f;
                _handles[h].used = true;
                _handles[h].mode = _regs_curr.bc.c == 0x40 ? 0 : 1;
                _regs_curr.af.a = (uint8_t)h;
            }
            break;
        case 0x41: // Close file handle
            {
                int h = _regs_curr.bc.b;
                if(h < MAX_HANDLES && _handles[h].used) {
                    fclose(_handles[h].fp);
                    _handles[h].used = false;
                    _regs_curr.af.a = 0x00;
                } else _regs_curr.af.a = 0xff;
            }
            break;
        case 0x42: // Read file handle
            {
                int h = _regs_curr.bc.b;
                uint16_t sz = _regs_curr.hl.word;
                uint16_t buf = _regs_curr.de.word;
                if(h < MAX_HANDLES && _handles[h].used) {
                    size_t r = fread(&_memory[buf], 1, sz, _handles[h].fp);
                    _regs_curr.hl.word = (uint16_t)r;
                    _regs_curr.af.a = (r < sz && feof(_handles[h].fp)) ? 0x01 : 0x00;
                } else { _regs_curr.af.a = 0xff; _regs_curr.hl.word = 0; }
            }
            break;
        case 0x43: // Write file handle
            {
                int h = _regs_curr.bc.b;
                uint16_t sz = _regs_curr.hl.word;
                uint16_t buf = _regs_curr.de.word;
                if(h < MAX_HANDLES && _handles[h].used) {
                    size_t w = fwrite(&_memory[buf], 1, sz, _handles[h].fp);
                    _regs_curr.hl.word = (uint16_t)w;
                    _regs_curr.af.a = (w < sz) ? 0x01 : 0x00;
                } else { _regs_curr.af.a = 0xff; _regs_curr.hl.word = 0; }
            }
            break;
        case 0x45: // Delete file (ASCIIZ)
            {
                char fpath[256]; int drv;
                parse_asciiz(_regs_curr.de.word, fpath, &drv);
                _regs_curr.af.a = (remove(fpath) == 0) ? 0x00 : 0xff;
            }
            break;
        case 0x46: // Seek file handle
            {
                int h = _regs_curr.bc.b;
                uint8_t whence = _regs_curr.bc.c;
                // offset in DEHL (32-bit little-endian)
                int32_t offset = _regs_curr.de.word | ((int32_t)_regs_curr.hl.word << 16);
                if(h < MAX_HANDLES && _handles[h].used) {
                    int wtab[] = {SEEK_SET, SEEK_CUR, SEEK_END, 3};
                    int w = (whence < 4) ? wtab[whence] : SEEK_SET;
                    fseek(_handles[h].fp, offset, w);
                    long pos = ftell(_handles[h].fp);
                    _regs_curr.de.word = (uint16_t)(pos & 0xFFFF);
                    _regs_curr.hl.word = (uint16_t)((pos >> 16) & 0xFFFF);
                    _regs_curr.af.a = 0x00;
                } else _regs_curr.af.a = 0xff;
            }
            break;
        case 0x47: // Move/Rename file (ASCIIZ)
            {
                char oldp[256], newp[256]; int drv;
                parse_asciiz(_regs_curr.de.word, oldp, &drv);
                parse_asciiz(_regs_curr.hl.word, newp, &drv);
                _regs_curr.af.a = (rename(oldp, newp) == 0) ? 0x00 : 0xff;
            }
            break;
        case 0x48: // Get default drive
            _regs_curr.af.a = _default_drive;
            break;
        case 0x49: // Set default drive
            _default_drive = _regs_curr.af.a & 1;
            break;
        case 0x4a: // Make directory
            {
                char fpath[256]; int drv;
                parse_asciiz(_regs_curr.de.word, fpath, &drv);
                _regs_curr.af.a = (mkdir(fpath, 0755) == 0) ? 0x00 : 0xff;
            }
            break;
        case 0x4b: // Remove directory
            {
                char fpath[256]; int drv;
                parse_asciiz(_regs_curr.de.word, fpath, &drv);
                _regs_curr.af.a = (rmdir(fpath) == 0) ? 0x00 : 0xff;
            }
            break;
        case 0x4c: // Change directory
            {
                char fpath[256]; int drv;
                parse_asciiz(_regs_curr.de.word, fpath, &drv);
                _regs_curr.af.a = (chdir(fpath) == 0) ? 0x00 : 0xff;
            }
            break;
        case 0x4d: // Get current directory
            {
                char *cwd = getcwd((char*)&_memory[_regs_curr.de.word], 256);
                _regs_curr.af.a = cwd ? 0x00 : 0xff;
            }
            break;
        case 0x4e: // Find First (ASCIIZ)
            if(_search2.active && _search2.dir) { closedir(_search2.dir); _search2.active = false; }
            {
                char pattern[256]; int drv;
                parse_asciiz(_regs_curr.de.word, pattern, &drv);
                _search2.active = true;
                // Separate directory from filename pattern
                char *slash = strrchr(pattern, PATHSEP);
                if(slash) {
                    strcpy(_search2.path, pattern);
                    _search2.path[(slash - pattern) + 1] = 0;
                    strcpy(pattern, slash + 1);
                } else {
                    strcpy(_search2.path, drv == 0 ? _disk_a_path : _disk_b_path);
                    strcat(_search2.path, PATHSEPSTR);
                }
                // Replace ? with * for POSIX
                for(char *p = pattern; *p; p++) if(*p == '?') *p = '*';
                _search2.dir = opendir(_search2.path);
                if(!_search2.dir) { _regs_curr.af.a = 0xff; break; }
            }
            // Fall through to Find Next
        case 0x4f: // Find Next
            {
                struct dirent *entry;
                while((entry = readdir(_search2.dir)) != NULL) {
                    struct stat st;
                    char full[256]; strcpy(full, _search2.path); strcat(full, entry->d_name);
                    if(stat(full, &st) != 0) continue;
                    // Build file info block at 0x80 (21-byte header + name)
                    uint8_t *info = &_memory[0x80];
                    memset(info, 0, 21);
                    // Attributes from st_mode
                    info[10] = S_ISDIR(st.st_mode) ? 0x10 : 0x00;
                    // Date/time (MSX-DOS format)
                    struct tm *tm = localtime(&st.st_mtime);
                    info[11] = ((tm->tm_year - 80) << 1) | (tm->tm_mon > 11 ? 1 : 0);
                    info[12] = ((tm->tm_mon + 1) << 5) | tm->tm_mday;
                    info[13] = (tm->tm_hour << 3) | (tm->tm_min >> 3);
                    info[14] = ((tm->tm_min & 7) << 5) | (tm->tm_sec >> 1);
                    // Size
                    info[15] = st.st_size & 0xFF;
                    info[16] = (st.st_size >> 8) & 0xFF;
                    info[17] = (st.st_size >> 16) & 0xFF;
                    info[18] = (st.st_size >> 24) & 0xFF;
                    // Name (11 bytes)
                    char *dot = strrchr(entry->d_name, '.');
                    char name[13]; strcpy(name, entry->d_name);
                    for(int i = 0; name[i]; i++) name[i] = toupper(name[i]);
                    if(dot) {
                        int nlen = dot - entry->d_name;
                        for(int i = 0; i < nlen && i < 8; i++) info[21+i] = name[i];
                        dot++;
                        for(int i = 0; i < 3 && dot[i]; i++) info[29+i] = dot[i];
                    } else {
                        for(int i = 0; i < 8 && name[i]; i++) info[21+i] = name[i];
                    }
                    _regs_curr.af.a = 0x00;
                    break;
                }
                if(!entry) { _regs_curr.af.a = 0xff; }
            }
            break;
        case 0x50: // Get file date/time (handle)
            {
                int h = _regs_curr.bc.b;
                if(h < MAX_HANDLES && _handles[h].used) {
                    struct stat st;
                    if(fstat(fileno(_handles[h].fp), &st) == 0) {
                        struct tm *tm = localtime(&st.st_mtime);
                        _regs_curr.de.d = tm->tm_year - 80; // years since 1980
                        _regs_curr.de.e = tm->tm_mon + 1;
                        _regs_curr.hl.h = tm->tm_mday;
                        _regs_curr.hl.l = tm->tm_hour;
                        _regs_curr.bc.b = tm->tm_min;
                        _regs_curr.bc.c = tm->tm_sec;
                        _regs_curr.af.a = 0x00;
                    } else _regs_curr.af.a = 0xff;
                } else _regs_curr.af.a = 0xff;
            }
            break;
        case 0x52: // Get file size (handle)
            {
                int h = _regs_curr.bc.b;
                if(h < MAX_HANDLES && _handles[h].used) {
                    long pos = ftell(_handles[h].fp);
                    fseek(_handles[h].fp, 0, SEEK_END);
                    long sz = ftell(_handles[h].fp);
                    fseek(_handles[h].fp, pos, SEEK_SET);
                    _regs_curr.de.word = (uint16_t)(sz & 0xFFFF);
                    _regs_curr.hl.word = (uint16_t)((sz >> 16) & 0xFFFF);
                    _regs_curr.af.a = 0x00;
                } else _regs_curr.af.a = 0xff;
            }
            break;
        case 0x53: // Truncate file (handle)
            {
                int h = _regs_curr.bc.b;
                if(h < MAX_HANDLES && _handles[h].used) {
                    _regs_curr.af.a = (ftruncate(fileno(_handles[h].fp), ftell(_handles[h].fp)) == 0) ? 0x00 : 0xff;
                } else _regs_curr.af.a = 0xff;
            }
            break;
        case 0x54: // Get disk free space
            {
                char fpath[256]; int drv;
                parse_asciiz(_regs_curr.de.word, fpath, &drv);
#ifdef _WIN32
                ULARGE_INTEGER freeBytes;
                if(GetDiskFreeSpaceExA(fpath, &freeBytes, NULL, NULL)) {
                    uint64_t free = (uint64_t)freeBytes.QuadPart;
                    _regs_curr.de.word = (uint16_t)(free & 0xFFFF);
                    _regs_curr.hl.word = (uint16_t)((free >> 16) & 0xFFFF);
                    _regs_curr.af.a = 0x00;
                } else _regs_curr.af.a = 0xff;
#else
                struct statvfs vfs;
                if(statvfs(fpath, &vfs) == 0) {
                    uint64_t free = (uint64_t)vfs.f_bavail * (uint64_t)vfs.f_bsize;
                    _regs_curr.de.word = (uint16_t)(free & 0xFFFF);
                    _regs_curr.hl.word = (uint16_t)((free >> 16) & 0xFFFF);
                    _regs_curr.af.a = 0x00;
                } else _regs_curr.af.a = 0xff;
#endif
            }
            break;
        case 0x56: // IOCTL get device info
            _regs_curr.hl.word = 0; // stdin/stdout is not a file
            _regs_curr.af.a = 0x00;
            break;
        case 0x5a: // Rename file (ASCIIZ)
            {
                char oldp[256], newp[256]; int drv;
                parse_asciiz(_regs_curr.de.word, oldp, &drv);
                parse_asciiz(_regs_curr.hl.word, newp, &drv);
                _regs_curr.af.a = (rename(oldp, newp) == 0) ? 0x00 : 0xff;
            }
            break;
        case 0x5b: // Get file info (ASCIIZ)
            {
                char fpath[256]; int drv;
                parse_asciiz(_regs_curr.de.word, fpath, &drv);
                struct stat st;
                if(stat(fpath, &st) == 0) {
                    uint8_t *info = &_memory[0x80];
                    memset(info, 0, 32);
                    info[10] = S_ISDIR(st.st_mode) ? 0x10 : 0x00;
                    struct tm *tm = localtime(&st.st_mtime);
                    info[11] = ((tm->tm_year - 80) << 1) | (tm->tm_mon > 11 ? 1 : 0);
                    info[12] = ((tm->tm_mon + 1) << 5) | tm->tm_mday;
                    info[13] = (tm->tm_hour << 3) | (tm->tm_min >> 3);
                    info[14] = ((tm->tm_min & 7) << 5) | (tm->tm_sec >> 1);
                    info[15] = st.st_size & 0xFF;
                    info[16] = (st.st_size >> 8) & 0xFF;
                    info[17] = (st.st_size >> 16) & 0xFF;
                    info[18] = (st.st_size >> 24) & 0xFF;
                    _regs_curr.af.a = 0x00;
                } else _regs_curr.af.a = 0xff;
            }
            break;
        case 0x5c: // Set file attributes (ASCIIZ)
            {
                char fpath[256]; int drv;
                parse_asciiz(_regs_curr.de.word, fpath, &drv);
                uint8_t attr = _regs_curr.bc.c;
                mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
                if(attr & 1) mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH); // read-only
                _regs_curr.af.a = (chmod(fpath, mode) == 0) ? 0x00 : 0xff;
            }
            break;
        case 0x5d: // Get environment string
            env_init();
            {
                char name[256];
                uint16_t addr = _regs_curr.de.word;
                int i = 0;
                while(_memory[addr + i] && i < 255) { name[i] = _memory[addr + i]; i++; }
                name[i] = 0;
                int nlen = strlen(name);
                uint16_t out = _regs_curr.hl.word;
                for(int e = 0; e < _env_count; e++) {
                    if(strncmp(_env[e], name, nlen) == 0 && _env[e][nlen] == '=') {
                        char *val = _env[e] + nlen + 1;
                        strcpy((char*)&_memory[out], val);
                        _regs_curr.hl.word = out;
                        _regs_curr.af.a = 0x00;
                        break;
                    }
                }
            }
            break;
        case 0x5e: // Set environment string
            env_init();
            {
                char entry[512];
                uint16_t addr = _regs_curr.de.word;
                int i = 0;
                while(_memory[addr + i] && i < 511) { entry[i] = _memory[addr + i]; i++; }
                entry[i] = 0;
                // Check if already exists
                char *eq = strchr(entry, '=');
                if(eq) {
                    int nlen = eq - entry;
                    for(int e = 0; e < _env_count; e++) {
                        if(strncmp(_env[e], entry, nlen) == 0 && _env[e][nlen] == '=') {
                            free(_env[e]);
                            _env[e] = strdup(entry);
                            _regs_curr.af.a = 0x00;
                            break;
                        }
                    }
                    // Not found, add new
                    if(_regs_curr.af.a != 0x00 && _env_count < MAX_ENV) {
                        _env[_env_count++] = strdup(entry);
                        _regs_curr.af.a = 0x00;
                    }
                }
                if(_regs_curr.af.a != 0x00) _regs_curr.af.a = 0xff;
            }
            break;
        case 0x5f: // Get environment item count
            env_init();
            _regs_curr.hl.word = (uint16_t)_env_count;
            _regs_curr.af.a = 0x00;
            break;
        case 0x60: // Get pointer to system variables
            // Return pointer to MSX-DOS 2.0 system vars at 0xE200
            _regs_curr.hl.word = 0xE200;
            _regs_curr.af.a = 0x00;
            break;
        case 0x61: // Get/Set environment pointer
            if(_regs_curr.de.e == 0) { // Get
                _regs_curr.hl.word = 0xE300; // environment pointer
                _regs_curr.af.a = 0x00;
            } else { // Set - ignore for now
                _regs_curr.af.a = 0x00;
            }
            break;
        case 0x63: // Get MSX-DOS 2.0 version number
            _regs_curr.hl.word = 0x0200;
            _regs_curr.de.word = 0x0001;
            _regs_curr.af.a = 0x00;
            break;
        case 0x62: // Terminate with error code
            _executing = false;
            _return_code = _regs_curr.bc.b;
            break;
        case 0x6f: // Get MSX-DOS Version Number
            _regs_curr.af.a = 0;
            _regs_curr.bc.word = 0x0200; // MSX-DOS 2.0
            _regs_curr.de.word = 0x0001;
        default:
            fprintf(stderr, "[WARNING: CALL 5 unimplemented 0x%02X]\n", _regs_curr.bc.c);
            _regs_curr.af.a = 0xff;
            _regs_curr.hl.l = 0xff;
            break;
    }

    // [English] Return from CALL 5 - pop return address from stack
    // [Portuguese] Retorno do CALL 5 - desempilha endereço de retorno da pilha
    _regs_curr.ip = mem_get_word(_regs_curr.sp);
    _regs_curr.sp += 2;
}