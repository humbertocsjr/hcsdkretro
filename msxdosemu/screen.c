#include "emu.h"

#define TTY_WIDTH 40
#define TTY_HEIGHT 24

static char _buffer[TTY_WIDTH * TTY_HEIGHT];
static int _x = 0;
static int _y = 0;
static int _put_mode = 0;
static bool _changed = true;

char printable(char c)
{
    if(c >= ' ' && c <= 127) return c;
    return ' ';
}

void screen_draw_reg(uint16_t curr, uint16_t prev, char *name, bool ptr, bool str)
{
    char curr_value[11];
    char prev_value[5];
    char name_fill[5];
    if(prev == curr) strcpy(prev_value, "----");
    else snprintf(prev_value, 5, "%04x", prev);
    snprintf(curr_value, 11, "%04x[%c|%c]", curr, printable(curr >> 8), printable(curr & 0xff));
    if(strlen(name) == 2)snprintf(name_fill, 5, " %s ", name);
    else snprintf(name_fill, 5, "%s    ", name);
    if(ptr && str)
    {
        printf(" %s| %s | %s | %04x'%c%c%c%c%c%c'", name_fill, prev_value, curr_value, mem_get_word(curr), printable(mem_get_byte(curr)), printable(mem_get_byte(curr + 1)), printable(mem_get_byte(curr + 2)), printable(mem_get_byte(curr + 3)), printable(mem_get_byte(curr + 4)), printable(mem_get_byte(curr + 5)));
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
    printf("\033[0m\033[0;37;40m\033[H\033[2J\033[3J");
    for(int y = 0; y < TTY_HEIGHT; y++)
    {
        printf("\033[0m\033[0;37;40m");
        for(int x = 0; x < TTY_WIDTH; x++)
        {
            char c = _buffer[y * TTY_WIDTH + x];
            if(c >= ' ' && c <= 127) printf("%c", c);
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
                printf("  ** | PREV | CURRENT   | [CURRENT]");
                break;
            case 7:
                if(!_debug) break;
                printf(" ----|------|-hhll-h-l--|-val--string--");
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
            case 18:
                if(!_debug) break;
                disasm(_regs_curr.ip);
                printf(" %04x %s%s", _regs_curr.ip, menu_color, _disasm);
                break;
            case 19:
                if(!_debug) break;
                printf(" CF:%c | S:%c | P/V:%c | N:%c | H:%c", cf_get() ? 'T':'F', sf_get() ? 'T':'F', pvf_get() ? 'T':'F', nf_get() ? 'T':'F', hf_get() ? 'T':'F');
                break;
            case 20:
                if(!_debug) break;
                if(_skip_call_step)printf(" STEP OVER RETURN ADDRESS:  %04x", _skip_call_step_address);
                break;
            case 21:
                if(!_debug) break;
                printf(" -= Use 'ld b,b' as breakpoint =-------");
                break;
            case 22:
                if(!_debug) break;
                printf(" -= CTRL+F10 = Export RAM [ram.bin] =--");
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
        return;
    }
    _changed = true;
    switch(c)
    {
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