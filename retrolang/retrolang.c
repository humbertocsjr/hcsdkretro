#include "retrolang.h"

char *_out_name = NULL;
FILE *_out = NULL;
include_path_t *_include_path = NULL;

void help()
{
    printf("RetroLang Compiler for Retro Computing v%d.%d R%d [%s]\n", VERSION, SUBVERSION, REVISION, cpu_name());
    printf("HC Software Development Kit for Retro Computing\n");
    printf("Copyright (c) 2025,2026 Humberto Costa dos Santos Junior\n\n");
    printf("Usage: retrolang-%s [ARGS] [SOURCE FILE]\n", cpu_ext());
    printf("Arguments:\n");
    printf("-o [FILE]       : Output Assembly file\n");
    printf("-I [PATH]       : Include path\n");
    exit(1);
}

int main(int argc, char **argv)
{
    char *src_name = NULL;
    for(int i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i], "-h"))
        {
            help();
        }
        else if(!strcmp(argv[i], "-o"))
        {
            i++;
            if(_out_name) error("output name already defined\n");
            if(i < argc) _out_name = argv[i];
        }
        else if(!strcmp(argv[i], "-I"))
        {
            i++;
            if(i < argc) 
            {
                include_path_t *inc = malloc(sizeof(include_path_t) + strlen(argv[i]));
                if(!inc) error("include path memory overflow.");
                memset(inc, 0, sizeof(include_path_t));
                inc->next = _include_path;
                _include_path = inc;
                strcpy(inc->path, argv[i]);
            }
        }
        else 
        {
            if(src_name) error("source file name already defined\n");
            src_name = argv[i];
        }
    }
    cpu_init();
    if(!_out_name) help();
    if(!src_name) help();
    _out = fopen(_out_name, "w");
    if(!_out) error("can't create file: %s", _out_name);

    parser_process_file(src_name);

    codegen();

    fclose(_out);
    _out = NULL;

    return 0;
}

