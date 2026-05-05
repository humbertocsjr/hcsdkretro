#include "../link.h"

static int _stack_size = 1024;
static int _max_mem = 0xF000;

// [English] Return the human-readable name of the output format
// [Portuguese] Retorna o nome legível do formato de saída
char *format_name()
{
    return "Flat Binary";
}

// [English] Return the file extension for the output format
// [Portuguese] Retorna a extensão de arquivo para o formato de saída
char *format_ext()
{
    return "bin";
}

// [English] Return the default output filename
// [Portuguese] Retorna o nome de arquivo de saída padrão
char *format_default_out_name()
{
    return "a.bin";
}

// [English] Print help text for format-specific command-line arguments
// [Portuguese] Imprime texto de ajuda para argumentos de linha de comando específicos do formato
void format_help_arguments()
{
    printf("-text [POSITION] : Set text/code position (default: 0)\n");
    printf("-data [POSITION] : Set data start position (default: after text)\n");
    printf("-bss [POSITION]  : Set bss start position (default: after data)\n");
    printf("-align [SIZE]    : Set section start align (default: 16)\n");
    printf("-stack [SIZE]    : Set stack size in bytes (default: 1024)\n");
    printf("-max-mem [SIZE]  : Set max memory available in bytes (default: 61440 = 60K)\n");
}

// [English] Parse format-specific command-line arguments
// [Portuguese] Analisa argumentos de linha de comando específicos do formato
bool format_parse_arg(int argc, int *argi, char **argv)
{
    // [English] Handle -text: set text section position
    // [Portuguese] Gerencia -text: define posição da seção de texto
    if (!strcmp(argv[*argi], "-text"))
    {
        (*argi)++;
        if (*argi < argc)
            section_find("text")->start_default_pos = toint(argv[*argi]);
        return true;
    }
    // [English] Handle -data: set data section position
    // [Portuguese] Gerencia -data: define posição da seção de dados
    else if (!strcmp(argv[*argi], "-data"))
    {
        (*argi)++;
        if (*argi < argc)
            section_find("data")->start_default_pos = toint(argv[*argi]);
        return true;
    }
    // [English] Handle -bss: set bss section position
    // [Portuguese] Gerencia -bss: define posição da seção bss
    else if (!strcmp(argv[*argi], "-bss"))
    {
        (*argi)++;
        if (*argi < argc)
            section_find("bss")->start_default_pos = toint(argv[*argi]);
        return true;
    }
    // [English] Handle -align: set default section alignment
    // [Portuguese] Gerencia -align: define alinhamento padrão de seção
    else if (!strcmp(argv[*argi], "-align"))
    {
        (*argi)++;
        if (*argi < argc)
            section_set_default_align(toint(argv[*argi]));
        return true;
    }
    // [English] Handle -stack: set stack size
    // [Portuguese] Gerencia -stack: define tamanho da pilha
    else if (!strcmp(argv[*argi], "-stack"))
    {
        (*argi)++;
        if (*argi < argc)
            _stack_size = toint(argv[*argi]);
        return true;
    }
    // [English] Handle -max-mem: set maximum memory available
    // [Portuguese] Gerencia -max-mem: define memória máxima disponível
    else if (!strcmp(argv[*argi], "-max-mem"))
    {
        (*argi)++;
        if (*argi < argc)
            _max_mem = toint(argv[*argi]);
        return true;
    }
    return false;
}

// [English] Initialize the flat binary format (no-op)
// [Portuguese] Inicializa o formato binário plano (sem operação)
void format_init()
{
}

// [English] Process all objects for the flat binary format, computing positions and generating output
// [Portuguese] Processa todos os objetos para o formato binário plano, computando posições e gerando saída
void format_process(step_t step)
{
    size_t position = 0;
    section_reset_sizes();
    // [English] Process the text section: align and compute position
    // [Portuguese] Processa a seção de texto: alinha e calcula posição
    section_t *sec = section_find("text");
    if (sec->start_default_pos == 0)
        sec->start_pos = (position + sec->align - 1) & (~(sec->align - 1));
    else
        sec->start_pos = sec->start_default_pos;
    position = process_objs(step, REC_SECTION_TEXT);

    // [English] Process the data section: align, pad with zeros if needed, then compute position
    // [Portuguese] Processa a seção de dados: alinha, preenche com zeros se necessário, depois calcula posição
    sec = section_find("data");
    if (sec->start_default_pos == 0)
        sec->start_pos = (position + sec->align - 1) & (~(sec->align - 1));
    else
        sec->start_pos = sec->start_default_pos;
    while (step == STEP_GENERATE && position < sec->start_pos)
    {
        outb(0);
        position++;
    }
    position = process_objs(step, REC_SECTION_DATA);

    // [English] Process the bss section: align and compute position
    // [Portuguese] Processa a seção bss: alinha e calcula posição
    sec = section_find("bss");
    if (sec->start_default_pos == 0)
        sec->start_pos = (position + sec->align - 1) & (~(sec->align - 1));
    else
        sec->start_pos = sec->start_default_pos;
    position = process_objs(step, REC_SECTION_BSS);

    // [English] Validate stack fits within available memory
    // [Portuguese] Valida se a pilha cabe na memória disponível
    int stack_top = (int)position + _stack_size;

    if (step == STEP_GENERATE && stack_top > _max_mem)
    {
        fprintf(stderr, "error: stack top (0x%X) exceeds max memory (0x%X)\n", stack_top, _max_mem);
        exit(1);
    }

    // [English] Set predefined link-time constants for sections and stack
    // [Portuguese] Define constantes predefinidas de tempo de link para seções e pilha
    consts_set(_objs, "__stack_top__", stack_top);
    consts_set(_objs, "__stack_size__", _stack_size);
    consts_set(_objs, "__text_start__", section_find("text")->start_pos);
    consts_set(_objs, "__data_start__", section_find("data")->start_pos);
    consts_set(_objs, "__bss_start__", section_find("bss")->start_pos);
    consts_set(_objs, "__text_end__", section_find("text")->position);
    consts_set(_objs, "_etext", section_find("text")->position);
    consts_set(_objs, "__data_end__", section_find("data")->position);
    consts_set(_objs, "_edata", section_find("data")->position);
    consts_set(_objs, "__bss_end__", section_find("bss")->position);
    consts_set(_objs, "_ebss", section_find("bss")->position);
    consts_set(_objs, "__text_size__", section_find("text")->size);
    consts_set(_objs, "__data_size__", section_find("data")->size);
    consts_set(_objs, "__bss_size__", section_find("bss")->size);
    consts_set(_objs, "_end", section_find("bss")->position);
}
