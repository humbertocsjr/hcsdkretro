#include "../link.h"

#define MZ_HEADER_SIZE 0x1C
#define MZ_HEADER_PARAS 2

static int _stack_size = 0x1000;
static int _max_mem = 0x10000;
static int _mz_text_size = 0;
static int _mz_data_size = 0;
static int _mz_bss_size = 0;

// [English] Write the MZ executable header (DOS EXE format)
// [Portuguese] Escreve o cabeçalho executável MZ (formato DOS EXE)
static void write_mz_header(int init_ss, int init_sp, int file_size)
{
    // [English] Compute paragraph counts for text and bss+stack
    // [Portuguese] Calcula contagens de parágrafos para texto e bss+pilha
    int text_para = MZ_HEADER_PARAS;
    int bss_stack_paras = (_mz_bss_size + _stack_size + 15) / 16;

    // [English] Write MZ signature and header fields
    // [Portuguese] Escreve assinatura MZ e campos do cabeçalho
    outb('M'); outb('Z');
    outw(file_size % 512);
    outw((file_size + 511) / 512);
    outw(0);
    outw(MZ_HEADER_PARAS);
    outw(bss_stack_paras);
    outw(0);
    outw(init_ss);
    outw(init_sp);
    outw(0);
    outw(0);
    outw(0);
    outw(MZ_HEADER_SIZE);
    outw(0);

    // [English] Pad header to the required paragraph boundary
    // [Portuguese] Preenche cabeçalho até o limite de parágrafo necessário
    for (int i = MZ_HEADER_SIZE; i < MZ_HEADER_PARAS * 16; i++)
        outb(0);
}

// [English] Return the human-readable name of the output format
// [Portuguese] Retorna o nome legível do formato de saída
char *format_name(void)
{
    return "MS-DOS MZ EXE (Independent Segments)";
}

// [English] Return the file extension for the output format
// [Portuguese] Retorna a extensão de arquivo para o formato de saída
char *format_ext(void)
{
    return "exe";
}

// [English] Return the default output filename
// [Portuguese] Retorna o nome de arquivo de saída padrão
char *format_default_out_name(void)
{
    return "a.exe";
}

// [English] Print help text for format-specific command-line arguments
// [Portuguese] Imprime texto de ajuda para argumentos de linha de comando específicos do formato
void format_help_arguments(void)
{
    printf("-stack [SIZE]    : Stack size in bytes (default: 4096)\n");
}

// [English] Parse format-specific command-line arguments for MZ format
// [Portuguese] Analisa argumentos de linha de comando específicos do formato MZ
bool format_parse_arg(int argc, int *argi, char **argv)
{
    // [English] Handle -stack: set stack segment size
    // [Portuguese] Gerencia -stack: define tamanho do segmento de pilha
    if (!strcmp(argv[*argi], "-stack"))
    {
        (*argi)++;
        if (*argi < argc)
            _stack_size = toint(argv[*argi]);
        return true;
    }
    // [English] Handle -max-mem: set maximum memory
    // [Portuguese] Gerencia -max-mem: define memória máxima
    if (!strcmp(argv[*argi], "-max-mem"))
    {
        (*argi)++;
        if (*argi < argc)
            _max_mem = toint(argv[*argi]);
        return true;
    }
    // [English] Silently consume unsupported arguments (-text, -data, -bss, -align)
    // [Portuguese] Consome silenciosamente argumentos não suportados (-text, -data, -bss, -align)
    if (!strcmp(argv[*argi], "-text") || !strcmp(argv[*argi], "-data") ||
        !strcmp(argv[*argi], "-bss") || !strcmp(argv[*argi], "-align"))
    {
        (*argi)++;
        if (*argi < argc)
            (*argi)++;
        return true;
    }
    return false;
}

// [English] Initialize the MZ format (no-op)
// [Portuguese] Inicializa o formato MZ (sem operação)
void format_init(void)
{
}

// [English] Process all objects for MZ format, computing segments and generating executable
// [Portuguese] Processa todos os objetos para o formato MZ, computando segmentos e gerando executável
void format_process(step_t step)
{
    section_reset_sizes();

    // [English] Seek past header area in generate step
    // [Portuguese] Avança além da área do cabeçalho na etapa de geração
    if (step == STEP_GENERATE)
        out_seek(MZ_HEADER_PARAS * 16, SEEK_SET);

    // [English] TEXT: starts at load module offset 0, file offset header_size
    // [Portuguese] TEXTO: começa no offset 0 do módulo de carga, offset do arquivo = tamanho do cabeçalho
    section_t *sec = section_find("text");
    sec->start_pos = 0;
    process_objs(step, REC_SECTION_TEXT);
    _mz_text_size = sec->size;

    // [English] DATA: separate segment, DS:0 is first DATA byte in memory.
    // [Portuguese] File bytes start after TEXT padding. Labels start at 0.
    // [English] DADOS: segmento separado, DS:0 é o primeiro byte de DADO na memória.
    // [Portuguese] Bytes do arquivo começam após padding do TEXTO. Rótulos começam em 0.
    sec = section_find("data");
    sec->start_pos = 0;
    {
        long file_want = MZ_HEADER_PARAS * 16 + ((_mz_text_size + 15) & ~15);
        if (step == STEP_GENERATE)
        {
            long file_pos = out_tell();
            while (file_pos < file_want)
            {
                outb(0);
                file_pos++;
            }
        }
    }
    process_objs(step, REC_SECTION_DATA);
    _mz_data_size = sec->size;

    // [English] BSS: separate segment, starts at SS:0. Not present in the file.
    // [Portuguese] BSS: segmento separado, começa em SS:0. Não está presente no arquivo.
    sec = section_find("bss");
    sec->start_pos = 0;
    process_objs(step, REC_SECTION_BSS);
    _mz_bss_size = sec->size;

    // [English] Compute segment deltas and validate stack fits in memory
    // [Portuguese] Calcula deltas de segmento e valida se a pilha cabe na memória
    {
        int text_paras = (_mz_text_size + 15) / 16;
        int data_paras = (_mz_data_size + 15) / 16;
        int data_seg_delta = text_paras;
        int bss_seg_delta = data_seg_delta + data_paras;
        int stack_top = _mz_bss_size + _stack_size;
        int init_ss = bss_seg_delta;
        int init_sp = stack_top;

        // [English] Write MZ header at the beginning of the file
        // [Portuguese] Escreve cabeçalho MZ no início do arquivo
        if (step == STEP_GENERATE)
        {
            if (stack_top > _max_mem)
            {
                fprintf(stderr, "error: stack top (0x%X) exceeds max memory (0x%X)\n",
                    stack_top, _max_mem);
                exit(1);
            }

            long file_end = out_tell();
            out_seek(0, SEEK_SET);
            write_mz_header(init_ss, init_sp, (int)file_end);
            out_seek(file_end, SEEK_SET);
        }

        // [English] Set segment delta constants for use by the program
        // [Portuguese] Define constantes de delta de segmento para uso pelo programa
        consts_set_global(_objs, "__data_seg_delta__");
        consts_set_global(_objs, "__bss_seg_delta__");
        consts_set(_objs, "__data_seg_delta__", data_seg_delta);
        consts_set(_objs, "__bss_seg_delta__", bss_seg_delta);
        consts_set(_objs, "__stack_size__", _stack_size);
        consts_set(_objs, "__stack_top__", stack_top);
    }

    // [English] Set predefined link-time constants for all sections
    // [Portuguese] Define constantes predefinidas de tempo de link para todas as seções
    consts_set(_objs, "__text_start__", section_find("text")->start_pos);
    consts_set(_objs, "__data_start__", 0);
    consts_set(_objs, "__bss_start__", 0);
    consts_set(_objs, "__text_end__", section_find("text")->position);
    consts_set(_objs, "_etext", section_find("text")->position);
    consts_set(_objs, "__data_end__", _mz_data_size);
    consts_set(_objs, "_edata", _mz_data_size);
    consts_set(_objs, "__bss_end__", _mz_bss_size);
    consts_set(_objs, "_ebss", _mz_bss_size);
    consts_set(_objs, "__text_size__", section_find("text")->size);
    consts_set(_objs, "__data_size__", _mz_data_size);
    consts_set(_objs, "__bss_size__", _mz_bss_size);
    consts_set(_objs, "_end", _mz_bss_size);
}
