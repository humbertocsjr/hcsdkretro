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

void screen_draw()
{
    printf("\033[H\033[2J\033[3J");
    for(int y = 0; y < TTY_HEIGHT; y++)
    {
        for(int x = 0; x < TTY_WIDTH; x++)
        {
            char c = _buffer[y * TTY_WIDTH + x];
            if(c >= ' ' && c <= 127) printf("%c", c);
            else printf(" ");
        }
        switch(y)
        {
            case 0:
                printf(" == MSX-DOS 1.0 Emulator == v%i.%i R%i", VERSION, SUBVERSION, REVISION);
                break;
            case 1:
                if(!_debug) printf(" -= CTRL+F9 = Enter Step Mode =-------");
                else printf(" -= CTRL+F9 = Exit Step Mode =--------");
                break;
            case 2:
                if(!_debug) break;
                printf(" -= CTRL+F12 = Step Into =------------");
                break;
            case 3:
                if(!_debug) break;
                printf(" -= CTRL+F11 = Step Over =------------");
                break;
            case 4:
                if(!_debug) break;
                printf(" -= CTRL+C = Exit =-------------------");
                break;
            case 5:
                if(!_debug) break;
                if(_regs_curr.af.word == _regs_prev.af.word) printf("   AF: %04x '%c'", _regs_curr.af.word, printable(_regs_curr.af.a));
                else printf("   AF: %04x '%c' [PREV: %04x]", _regs_curr.af.word, printable(_regs_curr.af.a), _regs_prev.af.word);
                break;
            case 6:
                if(!_debug) break;
                if(_regs_curr.bc.word == _regs_prev.bc.word) printf("   BC: %04x", _regs_curr.bc.word);
                else printf("   BC: %04x [PREV: %04x ]", _regs_curr.bc.word, _regs_prev.bc.word);
                break;
            case 7:
                if(!_debug) break;
                if(_regs_curr.de.word == _regs_prev.de.word) printf("   DE: %04x", _regs_curr.de.word);
                else printf("   DE: %04x [PREV: %04x ]", _regs_curr.de.word, _regs_prev.de.word);
                break;
            case 8:
                if(!_debug) break;
                if(_regs_curr.hl.word == _regs_prev.hl.word) printf("   HL: %04x", _regs_curr.hl.word);
                else printf("   HL: %04x [PREV: %04x ]", _regs_curr.hl.word, _regs_prev.hl.word);
                break;
            case 9:
                if(!_debug) break;
                if(_regs_curr.ix.word == _regs_prev.ix.word) printf("   IX: %04x", _regs_curr.ix.word);
                else printf("   IX: %04x [PREV: %04x ]", _regs_curr.ix.word, _regs_prev.ix.word);
                break;
            case 10:
                if(!_debug) break;
                if(_regs_curr.iy.word == _regs_prev.iy.word) printf("   IY: %04x", _regs_curr.iy.word);
                else printf("   IY: %04x [PREV: %04x ]", _regs_curr.iy.word, _regs_prev.iy.word);
                break;
            case 11:
                if(!_debug) break;
                if(_regs_curr.ip == _regs_prev.ip) printf("   IP: %04x", _regs_curr.ip);
                else printf("   IP: %04x [PREV: %04x ]", _regs_curr.ip, _regs_prev.ip);
                break;
            case 12:
                if(!_debug) break;
                if(_regs_curr.sp == _regs_prev.sp) printf("   SP: %04x", _regs_curr.sp);
                else printf("   SP: %04x [PREV: %04x ]", _regs_curr.sp, _regs_prev.sp);
                break;
            case 13:
                if(!_debug) break;
                printf(" [SP]: %04x '%c'", _memory[_regs_curr.sp] | (_memory[_regs_curr.sp + 1] << 8), printable(_memory[_regs_curr.sp]));
                break;
            case 14:
                if(!_debug) break;
                printf(" [HL]: %04x '%c'", _memory[_regs_curr.hl.word] | (_memory[_regs_curr.hl.word + 1] << 8), printable(_memory[_regs_curr.hl.word]));
                break;
            case 15:
                if(!_debug) break;
                printf(" [IX]: %04x '%c'", _memory[_regs_curr.ix.word] | (_memory[_regs_curr.ix.word + 1] << 8), printable(_memory[_regs_curr.ix.word]));
                break;
            case 16:
                if(!_debug) break;
                printf(" [IY]: %04x '%c'", _memory[_regs_curr.iy.word] | (_memory[_regs_curr.iy.word + 1] << 8), printable(_memory[_regs_curr.iy.word]));
                break;
            case 17:
                if(!_debug) break;
                disasm(_regs_curr.ip);
                printf(" [IP]: %s", _disasm);
                break;
            case 18:
                if(!_debug) break;
                if(_regs_curr.value == _regs_prev.value) printf("  VAL: %04x", _regs_curr.value);
                else printf("  VAL: %04x [PREV: %04x ]", _regs_curr.value, _regs_prev.value);
                break;
            case 20:
                if(!_debug) break;
                printf(" CF:%c | S:%c | P/V:%c | N:%c | H:%c", cf_get() ? 'T':'F', sf_get() ? 'T':'F', pvf_get() ? 'T':'F', nf_get() ? 'T':'F', hf_get() ? 'T':'F');
                break;
        }
        if(y < (TTY_HEIGHT - 1))printf("\n");
    }
    _changed = false;
}

void screen_draw_if_changed()
{
    if(_changed) screen_draw();
}

void screen_put_char(char c)
{
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