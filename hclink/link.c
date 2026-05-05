#include "link.h"

object_file_t *_objs = NULL;
object_file_t *_objs_last = NULL;
bool _multicpu = false;
rectype_t _cpu = 0;
bool _verbose = false;

// [English] Display help
// [Portuguese] Exibe ajuda
void help()
{
    printf("HC Linker for Retro Computing v%d.%d R%d [%s]\n", VERSION, SUBVERSION, REVISION, format_name());
    printf("HC Software Development Kit for Retro Computing\n");
    printf("Copyright (c) 2025,2026 Humberto Costa dos Santos Junior\n\n");
    printf("Usage: hclink-%s [ARGS] [OBJECT FILES ...]\n", format_ext());
    printf("Arguments:\n");
    printf("-o [FILE]       : Output file (default: a.bin)\n");
    printf("-sym [FILE]     : Symbol file\n");
    printf("-multicpu       : Ignore multiple object cpu types\n");
    printf("-v              : Verbose\n");
    format_help_arguments();
    exit(1);
}

// [English] Linker entry point
// [Portuguese] Ponto de entrada do linker
// [English] Parses arguments, initializes sections, processes objects, generates output
// [Portuguese] Analisa argumentos, inicializa seções, processa objetos, gera saída
int main(int argc, char **argv)
{
    char *out_name = NULL;
    char *sym_name = NULL;

    // [English] Create default sections
    // [Portuguese] Cria seções padrão
    section_new("text", NULL, REC_SECTION_TEXT);
    section_new("data", "text", REC_SECTION_DATA);
    section_new("bss", "data", REC_SECTION_BSS);
    section_new("reloc", "bss", REC_SECTION_RELOC);

    // [English] Parse command-line arguments
    // [Portuguese] Analisa argumentos da linha de comando
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-h"))
        {
            help();
        }
        else if (!strcmp(argv[i], "-v"))
        {
            _verbose = true;
        }
        else if (!strcmp(argv[i], "-sym"))
        {
            i++;
            if (sym_name)
                error("error: symbol file name already defined\n");
            if (i < argc)
                sym_name = argv[i];
        }
        else if (!strcmp(argv[i], "-o"))
        {
            i++;
            if (out_name)
                error("error: output name already defined\n");
            if (i < argc)
                out_name = argv[i];
        }
        else if (!strcmp(argv[i], "-multicpu"))
        {
            _multicpu = true;
        }
        else if (format_parse_arg(argc, &i, argv))
        {
            // Already done
        }
        else
        {
            obj_add(argv[i]);
        }
    }

    // [English] Set defaults if not specified
    // [Portuguese] Define padrões se não especificados
    if (!out_name)
        out_name = format_default_out_name();
    if (!_objs)
        help();

    // [English] Run linker pipeline
    // [Portuguese] Executa o pipeline do linker
    format_init();
    out_open(out_name);
    process(STEP_INITIALIZE);
    process(STEP_FILTER);
    process(STEP_PROCESS_FILES);
    reorder_start_first();
    process(STEP_VALIDATE);
    process(STEP_GENERATE);
    out_close();

    // [English] Print symbols if requested
    // [Portuguese] Imprime símbolos se solicitado
    if (sym_name)
        consts_print(sym_name);

    return 0;
}
