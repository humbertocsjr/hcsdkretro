#include "emu.h"

#ifdef _WIN32

static DWORD orig_console_mode;
char _keyb_keys[16];
char _keyb_keys_in = 0;
char _keyb_keys_out = 0;
char _keyb_keys_count = 0;

// [English] Initialize keyboard input (Windows raw mode)
// [Portuguese] Inicializa entrada do teclado (modo raw Windows)
void keyb_init()
{
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &orig_console_mode);
    SetConsoleMode(hStdin, orig_console_mode &
        ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT));
    atexit(keyb_exit);
}

// [English] Push a key into the circular buffer
// [Portuguese] Insere uma tecla no buffer circular
void keyb_push(char c)
{
    if(_keyb_keys_count >= 16) return;
    _keyb_keys[_keyb_keys_in] = c;
    _keyb_keys_in = (_keyb_keys_in + 1) % 16;
    _keyb_keys_count++;
}

// [English] Pop a key from the circular buffer
// [Portuguese] Remove uma tecla do buffer circular
bool keyb_pop(char *out)
{
    if(_keyb_keys_count == 0) return false;
    *out = _keyb_keys[_keyb_keys_out];
    _keyb_keys_out = (_keyb_keys_out + 1) % 16;
    _keyb_keys_count--;
    return true;
}

// [English] Check if a key is available
// [Portuguese] Verifica se há tecla disponível
bool keyb_avail()
{
    if(_kbhit()) return true;
    return _keyb_keys_count > 0;
}

// [English] Wait for and pop a key (blocking)
// [Portuguese] Aguarda e remove uma tecla (bloqueante)
char keyb_wait_pop()
{
    char c;
    while(!keyb_pop(&c))
    {
        keyb_process();
    }
    return c;
}

// [English] Restore original console mode on exit
// [Portuguese] Restaura modo original do console na saída
void keyb_exit()
{
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), orig_console_mode);
}

// [English] Get a single character from console (Windows)
// [Portuguese] Obtém um caractere do console (Windows)
static char keyb_getc()
{
    return _getch();
}

// [English] Process keyboard input (read keys, handle escape sequences for debug)
// [Portuguese] Processa entrada do teclado (lê teclas, gerencia sequências de escape para debug)
void keyb_process()
{
    char c = _kbhit() ? _getch() : 0;
    // [English] Handle escape sequences (cursor keys, function keys, etc.)
    // [Portuguese] Gerencia sequências de escape (teclas de cursor, teclas de função, etc.)
    if(c == 033)
    {
        int params[5];
        char seq[128];

        // [English] Read complete escape sequence
        // [Portuguese] Lê sequência de escape completa
        seq[0] = _kbhit() ? _getch() : 0;
        if(seq[0] == 0) return;
        for(int i = 1; i < 127; i++)
        {
            if(!_kbhit()) { seq[i] = 0; break; }
            seq[i] = _getch();
            seq[i+1] = 0;
            if(seq[i] == 0 || (seq[i] >= 0x40 && seq[i] <= 0x7f)) break;
        }

        // [English] Parse CSI sequences (ESC [ ... )
        // [Portuguese] Analisa sequências CSI (ESC [...)
        if(seq[0] == '[')
        {
            char *ptr = seq;
            ptr++;

            // [English] Parse numeric parameters separated by semicolons
            // [Portuguese] Analisa parâmetros numéricos separados por ponto e vírgula
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

            // [English] Dispatch debugger commands based on final character and parameters
            // [Portuguese] Despacha comandos do debugger baseado no caractere final e parâmetros
            switch(*ptr)
            {
                case '~':
                    switch(params[0])
                    {
                        // [English] Step into
                        // [Portuguese] Executar próxima instrução
                        case 24:
                        case 31:
                            if(_debug)
                            {
                                _next_step = true;
                            }
                            break;

                        // [English] Step over
                        // [Portuguese] Pular chamada
                        case 23:
                        case 29:
                            if(_debug)
                            {
                                _skip_call_step = true;
                                _next_step = true;
                                _skip_call_step_address = -1;
                            }
                            break;

                        // [English] Dump RAM and VRAM to files
                        // [Portuguese] Despeja RAM e VRAM para arquivos
                        case 21:
                        case 28:;
                            if(_debug)
                            {
                                FILE *ram_file = fopen("ram.bin", "wb");
                                if(ram_file)
                                {
                                    fwrite(_memory, 1, 0x10000, ram_file);
                                    fclose(ram_file); 
                                }
                                FILE *vram_file = fopen("vram.bin", "wb");
                                if(vram_file)
                                {
                                    fwrite(_vdp_memory, 1, VDP_MEMORY_MAX, vram_file);
                                    fclose(vram_file); 
                                }
                            }
                            break;

                        // [English] Toggle debug mode
                        // [Portuguese] Alterna modo debug
                        case 20:
                        case 26:
                            if(_debuggable)
                            {
                                _debug = !_debug;
                                if(_debug)
                                {
                                    _next_step = true;
                                }
                            }
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
    // [English] Regular key - push into buffer
    // [Portuguese] Tecla normal - insere no buffer
    else if(c)
    {
        keyb_push(c);
    }
}

#else /* POSIX */

#include <termios.h>
#include <unistd.h>

static struct termios orig_tio;
char _keyb_keys[16];
char _keyb_keys_in = 0;
char _keyb_keys_out = 0;
char _keyb_keys_count = 0;

// [English] Initialize keyboard input (POSIX raw mode)
// [Portuguese] Inicializa entrada do teclado (modo raw POSIX)
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

// [English] Push a key into the circular buffer
// [Portuguese] Insere uma tecla no buffer circular
void keyb_push(char c)
{
    if(_keyb_keys_count >= 16) return;
    _keyb_keys[_keyb_keys_in] = c;
    _keyb_keys_in = (_keyb_keys_in + 1) % 16;
    _keyb_keys_count++;
}

// [English] Pop a key from the circular buffer
// [Portuguese] Remove uma tecla do buffer circular
bool keyb_pop(char *out)
{
    if(_keyb_keys_count == 0) return false;
    *out = _keyb_keys[_keyb_keys_out];
    _keyb_keys_out = (_keyb_keys_out + 1) % 16;
    _keyb_keys_count--;
    return true;
}

// [English] Check if a key is available
// [Portuguese] Verifica se há tecla disponível
bool keyb_avail()
{
    return _keyb_keys_count > 0;
}

// [English] Wait for and pop a key (blocking)
// [Portuguese] Aguarda e remove uma tecla (bloqueante)
char keyb_wait_pop()
{
    char c;
    while(!keyb_pop(&c))
    {
        keyb_process();
    }
    return c;
}

// [English] Restore original terminal settings on exit
// [Portuguese] Restaura configurações originais do terminal na saída
void keyb_exit()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_tio);
}

// [English] Get a single character from console (POSIX)
// [Portuguese] Obtém um caractere do console (POSIX)
static char keyb_getc()
{
    char c;
    if(read(STDIN_FILENO, &c, 1) == 1) return c;
    return 0;
}

// [English] Process keyboard input (read keys, handle escape sequences for debug)
// [Portuguese] Processa entrada do teclado (lê teclas, gerencia sequências de escape para debug)
void keyb_process()
{
    char c = keyb_getc();

    // [English] Handle escape sequences (cursor keys, function keys, etc.)
    // [Portuguese] Gerencia sequências de escape (teclas de cursor, teclas de função, etc.)
    if(c == 033)
    {
        int params[5];
        char seq[128];

        // [English] Read complete escape sequence
        // [Portuguese] Lê sequência de escape completa
        seq[0] = keyb_getc();
        for(int i = 1; i < 127; i++)
        {
            seq[i] = keyb_getc();
            seq[i+1] = 0;
            if(seq[i] == 0 || (seq[i] >= 0x40 && seq[i] <= 0x7f)) break;
        }

        // [English] Parse CSI sequences (ESC [ ... )
        // [Portuguese] Analisa sequências CSI (ESC [...)
        if(seq[0] == '[')
        {
            char *ptr = seq;
            ptr++;

            // [English] Parse numeric parameters separated by semicolons
            // [Portuguese] Analisa parâmetros numéricos separados por ponto e vírgula
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

            // [English] Dispatch debugger commands based on final character and parameters
            // [Portuguese] Despacha comandos do debugger baseado no caractere final e parâmetros
            switch(*ptr)
            {
                case '~':
                    switch(params[0])
                    {
                        // [English] Step into
                        // [Portuguese] Executar próxima instrução
                        case 24:
                        case 31:
                            if(_debug)
                            {
                                _next_step = true;
                            }
                            break;

                        // [English] Step over
                        // [Portuguese] Pular chamada
                        case 23:
                        case 29:
                            if(_debug)
                            {
                                _skip_call_step = true;
                                _next_step = true;
                                _skip_call_step_address = -1;
                            }
                            break;

                        // [English] Dump RAM and VRAM to files
                        // [Portuguese] Despeja RAM e VRAM para arquivos
                        case 21:
                        case 28:;
                            if(_debug)
                            {
                                FILE *ram_file = fopen("ram.bin", "wb");
                                if(ram_file)
                                {
                                    fwrite(_memory, 1, 0x10000, ram_file);
                                    fclose(ram_file); 
                                }
                                FILE *vram_file = fopen("vram.bin", "wb");
                                if(vram_file)
                                {
                                    fwrite(_vdp_memory, 1, VDP_MEMORY_MAX, vram_file);
                                    fclose(vram_file); 
                                }
                            }
                            break;

                        // [English] Toggle debug mode
                        // [Portuguese] Alterna modo debug
                        case 20:
                        case 26:
                            if(_debuggable)
                            {
                                _debug = !_debug;
                                if(_debug)
                                {
                                    _next_step = true;
                                }
                            }
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
    // [English] Regular key - push into buffer
    // [Portuguese] Tecla normal - insere no buffer
    else if(c)
    {
        keyb_push(c);
    }
}

#endif