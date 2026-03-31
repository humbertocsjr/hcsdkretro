#include "emu.h"

#define TTY_WIDTH 40
#define TTY_HEIGHT 24
#define VDP_REGISTERS_MAX 8

char _vdp_memory[VDP_MEMORY_MAX];
static char *_buffer = &_vdp_memory[0];
static int _x = 0;
static int _y = 0;
static int _put_mode = 0;
static bool _changed = true;
static uint8_t _vdp_registers[VDP_MEMORY_MAX];
static uint8_t _vdp_curr_register = 0;
static uint8_t _vdp_first_byte = 0;
static bool _vdp_is_first_byte = true;
static uint16_t _vdp_pointer = 0;
static bool _vdp_enabled = true;

char printable(char c)
{
    if(c >= ' ' && c <= 127) return c;
    return ' ';
}

void screen_set_reg(uint8_t reg, uint8_t value)
{
    switch(reg & 0x3f)
    {
        case 0:

            break;
        case 1:
            _vdp_enabled = value & 2;
            break;
        case 2:

            break;
        case 3:

            break;
        case 4:
            _buffer = (char *)&_memory[(value & 0xf) * 0x400];
            break;
        case 5:

            break;
        case 6:

            break;
        case 7:

            break;
        default:
            fprintf(stderr, "[ERROR: VDP REGISTER NOT IMPLEMENTED: %i]", value & 0x3f);
            exit(1);
            break;
    }
    _vdp_registers[reg] = value;
}

void screen_out_99(uint8_t value) // Set Register
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

uint8_t screen_in_99() // Get Status
{
    _vdp_is_first_byte = true;
    return 0x00;
}

void screen_out_98(uint8_t value) // Data
{
    _vdp_memory[_vdp_pointer++ % VDP_MEMORY_MAX] = value;
}

uint8_t screen_in_98() // Data
{
    return _vdp_memory[_vdp_pointer++ % VDP_MEMORY_MAX];
}

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

void screen_draw()
{
    if(!_debuggable) return;
    char *title_color = "\033[0m\033[1;36;40m";
    char *menu_color = "\033[0m\033[0;36;40m";
    char *info_color = "\033[0m\033[0;37;40m";
    char *prev_color = "\033[0m\033[0;33;40m";
    char *next_color = "\033[0m\033[0;32;40m";
    char *true_text = "\033[0m\033[0;32;40mT\033[0m\033[0;37;40m";
    char *false_text = "\033[0m\033[0;31;40mF\033[0m\033[0;37;40m";

    printf("\033[0m\033[0;37;40m\033[H\033[2J\033[3J");
    for(int y = 0; y < TTY_HEIGHT; y++)
    {
        printf("\033[0m\033[0;37;40m");
        for(int x = 0; x < TTY_WIDTH; x++)
        {
            char c = _buffer[y * TTY_WIDTH + x];
            if(c >= ' ' && c <= 127) printf("%c", _vdp_enabled ? c : ' ');
            else printf(" ");
        }
        if(y == 0) printf("%s", title_color);
        else if(y < 5 || y > 20) printf("%s", menu_color);
        else printf("%s", info_color);
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
    printf("\033[%i;%iH", _y+1, _x+1);
    fflush(stdout);
    _changed = false;
}

void screen_draw_if_changed()
{
    if(_changed) screen_draw();
}

void screen_clear()
{
    printf("\033[H\033[2J\033[3J");
}

void screen_goto(int line, int column)
{
    _x = column - 1;
    _y = line - 1;
    if(_x >= TTY_WIDTH) _x %= TTY_WIDTH;
    if(_y >= TTY_HEIGHT) _y %= TTY_HEIGHT;
}

void screen_put_char(char c)
{
    if(!_debuggable)
    {
        switch(c)
        {
            case 12:
                screen_clear();
                break;
            default:
                printf("%c", c);
                break;
        }
        fflush(stdout);
        return;
    }
    _changed = true;
    switch(c)
    {
        case 8:
            if(_x == 0)
            {
                if(_y > 0)
                {
                    _x = TTY_WIDTH - 1;
                    _y--;
                }
            }
            else _x--;
            break;
        case 12:
            memset(_buffer, ' ', TTY_HEIGHT * TTY_WIDTH);
            _x = 0;
            _y = 0;
            break;
        case 13:
            _x = 0;
            break;
        case 10:
            _y++;
            break;
        default:
            _buffer[_y * TTY_WIDTH + _x] = c;
            _x++;
            break;
    }
    while(_x >= TTY_WIDTH)
    {
        _x -= TTY_WIDTH;
        _y++;
    }
    while(_y >= TTY_HEIGHT)
    {
        memmove(_buffer, &_buffer[TTY_WIDTH], TTY_WIDTH * (TTY_HEIGHT -1));
        memset(&_buffer[TTY_WIDTH * (TTY_HEIGHT -1)], ' ', TTY_WIDTH);
        _y--;
    }
}