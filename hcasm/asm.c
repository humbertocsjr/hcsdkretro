#include "asm.h"

source_t *_source = NULL;

void help()
{
    printf("HC Assembler for Retro Computing\n");
    printf("HC Software Development Kit for Retro Computing\n");
    printf("Copyright (c) 2025,2026 Humberto Costa dos Santos Junior\n\n");
    printf("Usage: hcasm-CPU [ARGS] [SOURCE FILE]\n");
    printf("Arguments:\n");
    printf("-o [FILE]       : Output file (default: a.obj)\n");
    printf("-dump [FILE]    : Dump description of object file (default: NONE)\n");
    exit(1);
}

int main(int argc, char **argv)
{
    char *out_name = NULL;
    char *dump_name = NULL;
    char *in_name = NULL;
    for(int i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i], "-h"))
        {
            help();
        }
        else if(!strcmp(argv[i], "-o"))
        {
            i++;
            if(out_name) error("output name already defined");
            if(i < argc) out_name = argv[i];
        }
        else if(!strcmp(argv[i], "-dump"))
        {
            i++;
            if(dump_name) error("dump name already defined");
            if(i < argc) dump_name = argv[i];
        }
        else 
        {
            if(in_name) error("source file name already defined");
            in_name = argv[i];
        }
    }
    if(!out_name) out_name = "a.obj";
    if(!in_name) help();
    out_open(out_name, dump_name);
    parse(in_name);
    out_close();
    return 0;
}

