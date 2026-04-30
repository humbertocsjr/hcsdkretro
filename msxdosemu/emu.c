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
int _return_code = 0;

void help()
{
    printf("MSX-DOS 1 Simplified Emulator for Retro Computing v%d.%d R%d\n", VERSION, SUBVERSION, REVISION);
    printf("HC Software Development Kit for Retro Computing\n");
    printf("Copyright (c) 2025,2026 Humberto Costa dos Santos Junior\n\n");
    printf("Usage: msxdosemu [EMULATOR ARGS] [PROG EXECUTABLE] [PROG ARGS]\n");
    printf("Examples:\n");
    printf("  msxdosemu -diska ./ cat.com test.txt\n");
    printf("  msxdosemu -diska ./ -debug cat.com test.txt\n");
    printf("  msxdosemu -diska ./ -step cat.com test.txt\n");
    printf("  msxdosemu -diska ./ -skip 0x103 cat.com test.txt\n");
    printf("Emulator Arguments:\n");
    printf("-debug          : Start on debug running mode\n");
    printf("-step           : Start on debug step mode\n");
    printf("-skip ADDRESS   : Start on debug step mode, step over to address\n");
    printf("-diska PATH     : Set Disk A Path\n");
    printf("-diskb PATH     : Set Disk B Path\n");
    exit(1);
}

int main(int argc, char **argv)
{
    char *com_name = NULL;
    char *sym_name = NULL;
    char *args_first_fname = NULL;
    char *args_second_fname = NULL;
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
                if(args[0]) strncat(args, " ", 128 - strlen(args) - 1);
                strncat(args, argv[i], 128 - strlen(args) - 1);
                if(!args_first_fname)
                {
                    args_first_fname = argv[i];
                }
                else if(!args_second_fname)
                {
                    args_second_fname = argv[i];
                }
            }
            else com_name = argv[i];
        }
    }
    memset(_memory, 0, 0x10000);
    if(!com_name) help();
    abi_dos_fcb_t *fcb = (abi_dos_fcb_t *)&_memory[0x5c];
    memset(fcb->name, ' ', 11);
    if(args_first_fname)
    {
        int ext_pos = -1;
        for (size_t i = 0; i < strlen(args_first_fname); i++)
        {
            if(ext_pos >= 0)
            {
                if(ext_pos < 3)
                {
                    fcb->ext[ext_pos++] = toupper(args_first_fname[i]);
                }
            }
            else if(i < 8)
            {
                if(args_first_fname[i] == '.')
                {
                    ext_pos = 0;
                }
                else
                {
                    fcb->name[i] = toupper(args_first_fname[i]);
                }
            }
        }
    }
    fcb = (abi_dos_fcb_t *)&_memory[0x6c];
    memset(fcb->name, ' ', 11);
    if(args_second_fname)
    {
        int ext_pos = -1;
        for (size_t i = 0; i < strlen(args_second_fname); i++)
        {
            if(ext_pos >= 0)
            {
                if(ext_pos < 3)
                {
                    fcb->ext[ext_pos++] = toupper(args_second_fname[i]);
                }
            }
            else if(i < 8)
            {
                if(args_second_fname[i] == '.')
                {
                    ext_pos = 0;
                }
                else
                {
                    fcb->name[i] = toupper(args_second_fname[i]);
                }
            }
        }
    }

    FILE *com_file = fopen(com_name, "rb");
    if(!com_file)
    {
        char *com_name_extended = malloc(strlen(com_name) + 5 + strlen(_disk_a_path));
        sprintf(com_name_extended, "%s%s", _disk_a_path, com_name);
        com_file = fopen(com_name_extended, "rb");
    }
    if(!com_file)
    {
        fprintf(stderr, "error: file not found: %s\n", com_name);
        return 1;
    }
    fread(&_memory[0x100], 1, 0xef00, com_file);
    fclose(com_file);

    // NOTE: ZEXDOC test skipping would go here
    // (requires understanding the ZEXDOC's internal test table structure)

    memcpy(&_memory[0x81], args, 127);
    _memory[0x80] = strlen(args) > 127 ? 127 : strlen(args);

    exec();
    return _return_code;
}

