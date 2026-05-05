#include "emu.h"

#define TTY_WIDTH 40
#define TTY_HEIGHT 24
#define VDP_REGISTERS_MAX 8

char _vdp_memory[VDP_MEMORY_MAX];
static char *_buffer = &_vdp_memory[0];
static int _x = 0;
static int _y = 0;
static int _put_mode = 0;
static int _esc_state = 0;
static uint8_t _esc_row = 0;
static bool _changed = true;
static uint8_t _vdp_registers[8];
static uint16_t _vdp_first_byte = 0;
static bool _vdp_is_first_byte = true;
static uint16_t _vdp_pointer = 0;
static bool _vdp_enabled = true;
static uint8_t _vdp_status = 0;
static uint8_t _vdp_foreground = 7;  // white
static uint8_t _vdp_background = 4;   // blue (MSX default)

// [English] Initialize the screen subsystem (enable ANSI on Windows)
// [Portuguese] Inicializa o subsistema de tela (habilita ANSI no Windows)
void screen_init()
{
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode;
        GetConsoleMode(hOut, &mode);
        SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#endif
}

// [English] MSX color to ANSI mapping
// [Portuguese] Mapeamento de cores MSX para ANSI
static const char *_vdp_ansi_fg[16] = {
    "\033[30m","\033[34m","\033[31m","\033[35m",
    "\033[32m","\033[36m","\033[33m","\033[37m",
    "\033[90m","\033[94m","\033[91m","\033[95m",
    "\033[92m","\033[96m","\033[93m","\033[97m"
};
static const char *_vdp_ansi_bg[16] = {
    "\033[40m","\033[44m","\033[41m","\033[45m",
    "\033[42m","\033[46m","\033[43m","\033[47m",
    "\033[100m","\033[104m","\033[101m","\033[105m",
    "\033[102m","\033[106m","\033[103m","\033[107m"
};

// [English] Return printable character or space if not printable
// [Portuguese] Retorna caractere imprimível ou espaço se não for imprimível
char printable(char c)
{
    if(c >= ' ' && c <= 127) return c;
    return ' ';
}

// [English] Set a VDP register
// [Portuguese] Define um registrador do VDP
void screen_set_reg(uint8_t reg, uint8_t value)
{
    _vdp_registers[reg] = value;
    switch(reg & 7)
    {
        case 0: // Mode + external video
            break;
        case 1: // Display on/off, mode, IE0
            _vdp_enabled = value & 0x40;
            break;
        case 2: // Name table base address (bits 4-6)
            break;
        case 3: // Color table (not used in text mode)
            break;
        case 4: // Pattern table base address
            _buffer = (char*)&_vdp_memory[(value & 7) * 0x800];
            break;
        case 5: // Sprite attribute table
            break;
        case 6: // Sprite pattern table
            break;
        case 7: // Foreground/background color
            _vdp_foreground = (value >> 4) & 0xF;
            _vdp_background = value & 0xF;
            break;
    }
}

// [English] Write to VDP register port 0x99
// [Portuguese] Escreve na porta de registrador VDP 0x99
void screen_out_99(uint8_t value)
{
    if(_vdp_is_first_byte)
    {
        _vdp_is_first_byte = false;
        _vdp_first_byte = value;
    }
    else if((value & 0x80) == 0)
    {
        _vdp_pointer = _vdp_first_byte | ((value & 0x3f) << 8);
        _vdp_is_first_byte = true;
    }
    else
    {
        screen_set_reg(value & 0x3f, _vdp_first_byte);
        _vdp_is_first_byte = true;
    }

}

// [English] Read VDP status from port 0x99
// [Portuguese] Lê status do VDP da porta 0x99
uint8_t screen_in_99()
{
    _vdp_is_first_byte = true;
    uint8_t s = _vdp_status;
    _vdp_status &= 0x7F; // clear interrupt flag on read
    return s;
}

// [English] Write data byte to VDP VRAM via port 0x98
// [Portuguese] Escreve byte de dados na VRAM do VDP via porta 0x98
void screen_out_98(uint8_t value)
{
    _vdp_memory[_vdp_pointer++ % VDP_MEMORY_MAX] = value;
}

// [English] Read data byte from VDP VRAM via port 0x98
// [Portuguese] Lê byte de dados da VRAM do VDP via porta 0x98
uint8_t screen_in_98()
{
    return _vdp_memory[_vdp_pointer++ % VDP_MEMORY_MAX];
}

// [English] Draw a single debug register line in the debug overlay
// [Portuguese] Desenha uma linha de registro no overlay de debug
void screen_draw_reg(uint16_t curr, uint16_t prev, char *name, bool ptr, bool str)
{
    char curr_value[11];
    char prev_value[5];
    char name_fill[5];
    if(prev == curr) strcpy(prev_value, "----");
    else snprintf(prev_value, 5, "%04x", prev);
    snprintf(curr_value, 11, "%04x %c %c", curr, printable(curr >> 8), printable(curr & 0xff));
    if(strlen(name) == 2)snprintf(name_fill, 5, " %s ", name);
    else snprintf(name_fill, 5, "%s    ", name);
    if(ptr && str)
    {
        printf(" %s| %s | %s | %04x'%c%c%c%c%c%c%c'", name_fill, prev_value, curr_value, mem_get_word(curr), printable(mem_get_byte(curr)), printable(mem_get_byte(curr + 1)), printable(mem_get_byte(curr + 2)), printable(mem_get_byte(curr + 3)), printable(mem_get_byte(curr + 4)), printable(mem_get_byte(curr + 5)), printable(mem_get_byte(curr + 6)));
    }
    else if(ptr)
    {
        printf(" %s| %s | %s | %04x ------", name_fill, prev_value, curr_value, mem_get_word(curr));
    }
    else
    {
        printf(" %s| %s | %s | ---- ------", name_fill, prev_value, curr_value);
    }
}

// [English] Full debug screen redraw
// [Portuguese] Redesenho completo da tela de debug
void screen_draw()
{
    if(!_debuggable) return;

    // [English] Define ANSI color codes
    // [Portuguese] Define códigos de cores ANSI
    char *title_color = "\033[0m\033[1;36;40m";
    char *menu_color = "\033[0m\033[0;36;40m";
    char *info_color = "\033[0m\033[0;37;40m";
    char *prev_color = "\033[0m\033[0;33;40m";
    char *next_color = "\033[0m\033[0;32;40m";
    char *true_text = "\033[0m\033[0;32;40mT\033[0m\033[0;37;40m";
    char *false_text = "\033[0m\033[0;31;40mF\033[0m\033[0;37;40m";

    // [English] Clear screen and render text buffer
    // [Portuguese] Limpa tela e renderiza buffer de texto
    printf("\033[0m%s%s\033[H\033[2J\033[3J", _vdp_ansi_bg[_vdp_background], _vdp_ansi_fg[_vdp_foreground]);
    for(int y = 0; y < TTY_HEIGHT; y++)
    {
        printf("\033[0m%s%s", _vdp_ansi_bg[_vdp_background], _vdp_ansi_fg[_vdp_foreground]);
        for(int x = 0; x < TTY_WIDTH; x++)
        {
            char c = _buffer[y * TTY_WIDTH + x];
            if(c >= ' ' && c <= 127) printf("%c", _vdp_enabled ? c : ' ');
            else printf(" ");
        }
        if(y == 0) printf("%s", title_color);
        else if(y < 5 || y > 20) printf("%s", menu_color);
        else printf("%s", info_color);

        // [English] Draw info/status lines
        // [Portuguese] Desenha linhas de informação/status
        switch(y)
        {
            case 0:
                printf(" == MSX-DOS 1.0 Emulator ==== HC SDK ==");
                break;
            case 1:
                if(!_debug) printf(" -= CTRL/ALT+F9 = Enter Step Mode =----");
                else printf(" -= CTRL/ALT+F9 = Continue =-----------");
                break;
            case 2:
                if(!_debug) break;
                printf(" -= CTRL/ALT+F12 = Step Into =---------");
                break;
            case 3:
                if(!_debug) break;
                printf(" -= CTRL/ALT+F11 = Step Over =---------");
                break;
            case 4:
                if(!_debug) break;
                printf(" -= CTRL+C = Exit =--------------------");
                break;
            case 6:
                if(!_debug) break;
                printf("  ** | PREV | CURRENT  | [CURRENT]");
                break;
            case 7:
                if(!_debug) break;
                printf(" ----|------|-hhll-h-l-|-val--string---");
                break;
            case 8:
                if(!_debug) break;
                screen_draw_reg(_regs_curr.af.word, _regs_prev.af.word, "AF", false, false);
                break;
            case 9:
                if(!_debug) break;
                screen_draw_reg(_regs_curr.bc.word, _regs_prev.bc.word, "BC", true, true);
                break;
            case 10:
                if(!_debug) break;
                screen_draw_reg(_regs_curr.de.word, _regs_prev.de.word, "DE", true, true);
                break;
            case 11:
                if(!_debug) break;
                screen_draw_reg(_regs_curr.hl.word, _regs_prev.hl.word, "HL", true, true);
                break;
            case 12:
                if(!_debug) break;
                screen_draw_reg(_regs_curr.ix.word, _regs_prev.ix.word, "IX", true, true);
                break;
            case 13:
                if(!_debug) break;
                screen_draw_reg(_regs_curr.iy.word, _regs_prev.iy.word, "IY", true, true);
                break;
            case 14:
                if(!_debug) break;
                screen_draw_reg(_regs_curr.sp, _regs_prev.sp, "SP", true, false);
                break;
            case 15:
                if(!_debug) break;
                screen_draw_reg(_regs_curr.ip, _regs_prev.ip, "IP", true, false);
                break;
            case 16:
                if(!_debug) break;
                screen_draw_reg(_regs_curr.value, _regs_prev.value, "->", true, true);
                break; 
            case 17:
                if(!_debug) break;
                if(_skip_call_step)printf(" STEP OVER RETURN ADDRESS:  %04x", _skip_call_step_address);
                break;
            case 18:
                if(!_debug) break;
                disasm(_regs_prev.ip);
                printf(" %sPREV%s[ %04x ] %s%s", prev_color, info_color, _regs_prev.ip, prev_color, _disasm);
                break;
            case 19:
                if(!_debug) break;
                disasm(_regs_curr.ip);
                printf(" %sNEXT%s[ %04x ] %s%s", next_color, info_color, _regs_curr.ip, next_color, _disasm);
                break;
            case 20:
                if(!_debug) break;
                printf(" CF:%s | S:%s | P/V:%s | N:%s | H:%s | Z:%s", cf_get() ? true_text : false_text, sf_get() ? true_text : false_text, pvf_get() ? true_text : false_text, nf_get() ? true_text : false_text, hf_get() ? true_text : false_text, zf_get() ? true_text : false_text);
                break;
            case 21:
                if(!_debug) break;
                printf(" -= Use 'ld b,b' as breakpoint =-------");
                break;
            case 22:
                if(!_debug) break;
                printf(" -= CTRL/ALT+F10 = Dump RAM/VRAM =----");
                break;
            case 23:
                if(!_debug) break;
                printf(" ----------------= ram.bin/vram.bin =-");
                break;
        }
        if(y < (TTY_HEIGHT - 1))
        {
            printf("\033[0m\n");
        }
    }
    // [English] Restore cursor position to match emulated terminal
    // [Portuguese] Restaura posição do cursor para corresponder ao terminal emulado
    printf("\033[%i;%iH", _y+1, _x+1);
    fflush(stdout);
    _changed = false;
}

// [English] Redraw debug screen only if content changed
// [Portuguese] Redesenha tela de debug apenas se houve mudança
void screen_draw_if_changed()
{
    if(_changed) screen_draw();
}

// [English] Clear the screen
// [Portuguese] Limpa a tela
void screen_clear()
{
    printf("\033[H\033[2J\033[3J");
}

// [English] Move cursor to specified position (1-based)
// [Portuguese] Move cursor para posição especificada (base 1)
void screen_goto(int line, int column)
{
    _x = column - 1;
    _y = line - 1;
    if(_x >= TTY_WIDTH) _x %= TTY_WIDTH;
    if(_y >= TTY_HEIGHT) _y %= TTY_HEIGHT;
}

// [English] Scroll the text buffer up one line
// [Portuguese] Rola o buffer de texto uma linha para cima
static void screen_scroll()
{
    memmove(_buffer, &_buffer[TTY_WIDTH], TTY_WIDTH * (TTY_HEIGHT -1));
    memset(&_buffer[TTY_WIDTH * (TTY_HEIGHT -1)], ' ', TTY_WIDTH);
    _y--;
}

// [English] Output a character to the screen (handles escape sequences, control chars)
// [Portuguese] Envia um caractere para a tela (gerencia sequências de escape, caracteres de controle)
void screen_put_char(char c)
{
    // [English] Simple mode (no debug overlay)
    // [Portuguese] Modo simples (sem overlay de debug)
    if(!_debuggable)
    {
        switch(c)
        {
            case 7:  printf("\a"); break;
            case 8:  printf("\b \b"); break;
            case 9:  printf("\t"); break;
            case 12: screen_clear(); break;
            default: printf("%c", c); break;
        }
        fflush(stdout);
        return;
    }
    _changed = true;

    // [English] Handle escape sequences statefully
    // [Portuguese] Gerencia sequências de escape com estado
    if(_esc_state == 1) {
        if(c == 'Y') { _esc_state = 2; return; }    // ESC Y r c - cursor position
        if(c == 'K') { for(int cx = _x; cx < TTY_WIDTH; cx++) _buffer[_y * TTY_WIDTH + cx] = ' '; _esc_state = 0; return; }
        if(c == 'J') { for(int cy = _y; cy < TTY_HEIGHT; cy++) for(int cx = (cy==_y?_x:0); cx < TTY_WIDTH; cx++) _buffer[cy * TTY_WIDTH + cx] = ' '; _esc_state = 0; return; }
        if(c == 'H') { _x = 0; _y = 0; _esc_state = 0; return; }
        if(c == 'E') { memset(_buffer, ' ', TTY_HEIGHT * TTY_WIDTH); _x = 0; _y = 0; _esc_state = 0; return; }
        _esc_state = 0; // unknown sequence, abort
    }
    if(_esc_state == 2) { _esc_row = c - 32; _esc_state = 3; return; }
    if(_esc_state == 3) { _y = _esc_row; _x = c - 32; _esc_state = 0; return; }

    // [English] Process control characters and regular characters
    // [Portuguese] Processa caracteres de controle e regulares
    switch(c)
    {
        case 7:  // BEL - bell
            printf("\a");
            fflush(stdout);
            break;
        case 8:  // BS - backspace
            if(_x == 0) { if(_y > 0) { _x = TTY_WIDTH - 1; _y--; } }
            else _x--;
            break;
        case 9:  // HT - horizontal tab
            _x = (_x + 8) & ~7;
            break;
        case 10: // LF - line feed
            _y++;
            break;
        case 11: // VT - vertical tab
            _y++;
            break;
        case 12: // FF - form feed (clear screen)
            memset(_buffer, ' ', TTY_HEIGHT * TTY_WIDTH);
            _x = 0; _y = 0;
            break;
        case 13: // CR - carriage return
            _x = 0;
            break;
        case 27: // ESC - start escape sequence
            _esc_state = 1;
            break;
        case 127: // DEL - delete (ignored)
            break;
        default: // Regular character / Caractere regular
            _buffer[_y * TTY_WIDTH + _x] = c;
            _x++;
            break;
    }
    // [English] Handle line wrapping and scrolling
    // [Portuguese] Gerencia quebra de linha e rolagem
    while(_x >= TTY_WIDTH) { _x -= TTY_WIDTH; _y++; }
    while(_y >= TTY_HEIGHT) screen_scroll();
}