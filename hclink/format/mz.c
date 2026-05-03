#include "../link.h"

#define MZ_HEADER_SIZE 0x1C
#define MZ_HEADER_PARAS 2

static int _stack_size = 0x400;
static int _mz_data_start = 0;
static int _mz_text_size = 0;
static int _mz_data_size = 0;
static int _mz_bss_size = 0;

static int mz_adjust_value(int value, int position, rectype_t section)
{
    if (_mz_data_start > 0 && value >= _mz_data_start)
        return value - _mz_data_start;
    return value;
}

static void mz_record_reloc(int position, rectype_t section)
{
}

static void write_mz_header(void)
{
    int text_para = MZ_HEADER_PARAS;
    int data_para = text_para + (_mz_text_size + 15) / 16;
    int bss_para = (_mz_bss_size + 15) / 16;
    int stack_size_padded = (_stack_size + 15) & ~15;
    int bss_stack_paras = bss_para + stack_size_padded / 16;

    int file_size = MZ_HEADER_PARAS * 16 + ((_mz_text_size + 15) & ~15) + ((_mz_data_size + 15) & ~15);

    outb('M'); outb('Z');
    outw(file_size % 512);
    outw((file_size + 511) / 512);
    outw(0);
    outw(MZ_HEADER_PARAS);
    outw(bss_stack_paras);
    outw(0xFFFF);
    outw(data_para);
    outw(_mz_data_size + _mz_bss_size + _stack_size);
    outw(0);
    outw(0);
    outw(text_para);
    outw(MZ_HEADER_SIZE);
    outw(0);

    for (int i = MZ_HEADER_SIZE; i < MZ_HEADER_PARAS * 16; i++)
        outb(0);
}

char *format_name(void)
{
    return "MS-DOS MZ EXE";
}

char *format_ext(void)
{
    return "exe";
}

char *format_default_out_name(void)
{
    return "a.exe";
}

void format_help_arguments(void)
{
    printf("-stack [SIZE]    : Stack size in bytes (default: 1024)\n");
}

bool format_parse_arg(int argc, int *argi, char **argv)
{
    if (!strcmp(argv[*argi], "-stack"))
    {
        (*argi)++;
        if (*argi < argc)
            _stack_size = toint(argv[*argi]);
        return true;
    }
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

void format_init(void)
{
}

void format_process(step_t step)
{
    section_reset_sizes();

    if (step == STEP_GENERATE)
    {
        format_adjust_value = mz_adjust_value;
        format_record_reloc = mz_record_reloc;
        out_seek(MZ_HEADER_PARAS * 16, SEEK_SET);
    }

    section_t *sec = section_find("text");
    sec->start_pos = MZ_HEADER_PARAS * 16;
    size_t position = process_objs(step, REC_SECTION_TEXT);
    _mz_text_size = position - sec->start_pos;

    sec = section_find("data");
    sec->start_pos = (position + 15) & ~15;
    while (step == STEP_GENERATE && position < sec->start_pos)
    {
        outb(0);
        position++;
    }
    position = process_objs(step, REC_SECTION_DATA);
    _mz_data_size = position - sec->start_pos;

    sec = section_find("bss");
    sec->start_pos = (position + 15) & ~15;
    position = process_objs(step, REC_SECTION_BSS);
    _mz_bss_size = position - sec->start_pos;

    _mz_data_start = section_find("data")->start_pos;

    if (step == STEP_GENERATE)
    {
        format_adjust_value = NULL;
        format_record_reloc = NULL;

        long file_end = out_tell();
        out_seek(0, SEEK_SET);
        write_mz_header();
        out_seek(file_end, SEEK_SET);
    }

    {
        int text_para = MZ_HEADER_PARAS;
        int data_para = text_para + (_mz_text_size + 15) / 16;
        int bss_offset = ((_mz_data_size + 15) & ~15);
        int stack_top = bss_offset + _mz_bss_size + _stack_size;
        consts_set(_objs, "__cs_seg__", text_para);
        consts_set(_objs, "__data_seg_delta__", data_para - text_para);
        consts_set(_objs, "__stack_size__", _stack_size);
        consts_set(_objs, "__stack_top__", stack_top);
    }

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
