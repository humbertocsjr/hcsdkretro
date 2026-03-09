#include "../link.h"

char *format_name()
{
    return "Relocatable Executable";
}

char *format_ext()
{
    return "rex";
}

char *format_default_out_name()
{
    return "a.rex";
}

void format_help_arguments()
{
    printf("-align [SIZE]   : Set section start align (default: 16)\n");
}

bool format_parse_arg(int argc, int *argi, char **argv)
{
    if(!strcmp(argv[*argi], "-align"))
    {
        (*argi)++;
        if(*argi < argc)
        {
            section_set_default_align(toint(argv[*argi]));
        }
        return true;
    }
    return false;
}

void format_init()
{

}

void format_process(step_t step)
{
    size_t position = 0;
    if(step == STEP_GENERATE)
    {
        outb('H');
        outb('C');
        outw(section_find("text")->size);
        outw(section_find("data")->size);
        outw(section_find("bss")->size);
        outw(consts_get(NULL, "_start"));
        outw(section_find("reloc")->size);
        outw(0);
        outw(_cpu << 8);
    }
    section_reset_sizes();
    section_t *sec = section_find("text");
    if(sec->start_default_pos == 0)
    {
        sec->start_pos = (position + sec->align - 1) & (~(sec->align - 1));
    }
    else sec->start_pos = sec->start_default_pos;
    position = process_objs(step, REC_SECTION_TEXT);
    sec = section_find("data");
    if(sec->start_default_pos == 0)
    {
        sec->start_pos = (position + sec->align - 1) & (~(sec->align - 1));
    }
    else sec->start_pos = sec->start_default_pos;
    while(step == STEP_GENERATE && position < sec->start_pos)
    {
        outb(0);
        position++;
    }
    position = process_objs(step, REC_SECTION_DATA);
    sec = section_find("bss");
    if(sec->start_default_pos == 0)
    {
        sec->start_pos = (position + sec->align - 1) & (~(sec->align - 1));
    }
    else sec->start_pos = sec->start_default_pos;
    position = process_objs(step, REC_SECTION_BSS);
    process_objs(step, REC_SECTION_RELOC);
    consts_set(_objs, "__text_start__", section_find("text")->start_pos);
    consts_set(_objs, "__data_start__", section_find("data")->start_pos);
    consts_set(_objs, "__bss_start__", section_find("bss")->start_pos);
    consts_set(_objs, "__text_end__", section_find("text")->position);
    consts_set(_objs, "__data_end__", section_find("data")->position);
    consts_set(_objs, "__bss_end__", section_find("bss")->position);
    consts_set(_objs, "__text_size__", section_find("text")->size);
    consts_set(_objs, "__data_size__", section_find("data")->size);
    consts_set(_objs, "__bss_size__", section_find("bss")->size);
    consts_set(_objs, "_end", section_find("bss")->position);
}