#include "emu.h"

bool _debuggable = false;
bool _debug = false;
uint8_t _memory[0x100ff];
z80_regs_t _regs_curr;
z80_regs_t _regs_prev;
bool _executing = true;
bool _next_step = false;
bool _skip_call_step = false;
int32_t _skip_call_step_address = 0;
char *_disk_a_path = "";
char *_disk_b_path = "";
uint16_t _disk_transferr_address = 0x80;

void help()
{
    printf("MSX-DOS 1 Simplified Emulator for Retro Computing v%d.%d R%d\n", VERSION, SUBVERSION, REVISION);
    printf("HC Software Development Kit for Retro Computing\n");
    printf("Copyright (c) 2025,2026 Humberto Costa dos Santos Junior\n\n");
    printf("Usage: msxdosemu [-step] [-sym FILE] [EXECUTABLE] [ARGS]\n");
    printf("Arguments:\n");
    printf("-debug          : Start on debug running mode\n");
    printf("-step           : Start on debug step mode\n");
    printf("-skip ADDRESS   : Start on debug step mode, step over to address\n");
    printf("-sym            : Read Symbols File\n");
    printf("-diska PATH     : Set Disk A Path\n");
    printf("-diskb PATH     : Set Disk B Path\n");
    exit(1);
}

int main(int argc, char **argv)
{
    char *com_name = NULL;
    char *sym_name = NULL;
    char args[128];
    char *endptr;
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
                fprintf(stderr, "error: symbol file already defined.\n");
                return 1;
            }
            i++;
            if(i < argc) sym_name = argv[1];
        }
        else if(!strcmp(argv[i], "-diska"))
        {
            i++;
            if(i < argc) _disk_a_path = argv[i];
        }
        else if(!strcmp(argv[i], "-diskb"))
        {
            i++;
            if(i < argc) _disk_b_path = argv[i];
        }
        else if(!strcmp(argv[i], "-debug"))
        {
            _debuggable = true;
        }
        else if(!strcmp(argv[i], "-step"))
        {
            _debug = true;
            _debuggable = true;
        }
        else if(!strcmp(argv[i], "-skip"))
        {
            _debuggable = true;
            _debug = true;
            _next_step = true;
            _skip_call_step = true;
            i++;
            if(i >= argc)
            {
                fprintf(stderr, "error: skip address expected.\n");
                return 1;
            }
            _skip_call_step_address = strtol(argv[i], &endptr, 0);
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
    if(!com_name) help();
    FILE *com_file = fopen(com_name, "rb");
    if(!com_file)
    {
        fprintf(stderr, "error: file not found: %s\n", com_name);
        return 1;
    }
    fread(&_memory[0x100], 1, 0xef00, com_file);
    fclose(com_file);

    exec();
    return 0;
}

