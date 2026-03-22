#include "emu.h"

bool _debug = false;
uint8_t _memory[0x10000];
z80_regs_t _regs_curr;
z80_regs_t _regs_prev;
bool _executing = true;

void help()
{
    printf("MSX-DOS 1 Simplified Emulator for Retro Computing v%d.%d R%d\n", VERSION, SUBVERSION, REVISION);
    printf("HC Software Development Kit for Retro Computing\n");
    printf("Copyright (c) 2025,2026 Humberto Costa dos Santos Junior\n\n");
    printf("Usage: msxdosemu [-step] [-sym FILE] [EXECUTABLE] [ARGS]\n");
    printf("Arguments:\n");
    printf("-step           : Start on step mode\n");
    printf("-sym            : Read Symbols File\n");
    exit(1);
}

int main(int argc, char **argv)
{
    char *com_name = NULL;
    char *sym_name = NULL;
    char args[128];
    strcpy(args, "");
    for(int i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i], "-h"))
        {
            help();
        }
        else if(!strcmp(argv[i], "-sym"))
        {
            if(sym_name) 
            {
                fprinf(stderr, "error: symbol file already defined.\n");
                return 1;
            }
            i++;
            if(i < argc) sym_name = argv[1];
        }
        else if(!strcmp(argv[i], "-step"))
        {
            _debug = true;
        }
        else 
        {
            if(com_name)
            {
                strncpy(args, argv[i], 128 - strlen(args));
                strncpy(args, " ", 128 - strlen(args));
            }
            else com_name = argv[i];
        }
    }
    memset(_memory, 0, 0x10000);
    FILE *com_file = fopen(com_name, "rb");
    fread(&_memory[0x100], 1, 0xef00, com_file);
    fclose(com_file);

    exec();
    return 0;
}

