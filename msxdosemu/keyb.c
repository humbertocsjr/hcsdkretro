#include "emu.h"

#include <termios.h>
#include <unistd.h>

static struct termios orig_tio;
char _keyb_key = 0;

void keyb_init()
{
    struct termios new_tio;
    tcgetattr(STDIN_FILENO, &orig_tio);
    atexit(keyb_exit);
    new_tio = orig_tio;
    new_tio.c_lflag &= (~ICANON & ~ECHO); 
    new_tio.c_cc[VMIN] = 0;
    new_tio.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void keyb_exit()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_tio);
}

static char keyb_getc()
{
    char c;
    if(read(STDIN_FILENO, &c, 1) == 1) return c;
    return 0;
}

void keyb_process()
{
    char c = keyb_getc();
    if(c != 0) _keyb_key = c;
    if(c == 033)
    {
        int params[5];
        char seq[128];
        seq[0] = keyb_getc();
        for(int i = 1; i < 127; i++)
        {
            seq[i] = keyb_getc();
            seq[i+1] = 0;
            if(seq[i] == 0 || (seq[i] >= 0x40 && seq[i] <= 0x7f)) break;
        }
        _keyb_key = 0;
        if(seq[0] == '[')
        {
            char *ptr = seq;
            ptr++;
            int params_total = 0;
            for(int i = 0; i < 5; i++)
            {
                params[i] = 0;
                while(*ptr == ' ') ptr++;
                if(*ptr >= '0' && *ptr <= '9') params_total++;
                while(*ptr)
                {
                    if(*ptr >= '0' && *ptr <= '9')
                    {
                        params[i] *= 10;
                        params[i] += *ptr - '0';
                    }
                    else break;
                    ptr++;
                }
                if(*ptr != ';') break;
                ptr++;
            }
            switch(*ptr)
            {
                case '~':
                    switch(params[0])
                    {
                        case 24:
                            _next_step = true;
                            break;
                        case 23:
                            _skip_call_step = true;
                            _next_step = true;
                            break;
                        case 20:
                            _debug = !_debug;
                            break;
                        default:
                            printf("\r{ESC%s  -> %c %i %i}", seq, *ptr, params[0], params[1]);
                            exit(1);
                            break;
                    }
                    break;
            }
        }
        else
        {
            printf("{ESC%s}", seq);
            exit(1);
        }
    }
}
