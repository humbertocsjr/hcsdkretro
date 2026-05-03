#include "../link.h"

#define MZ_HEADER_SIZE 0x1C
#define MZ_HEADER_PARAS 2

static int _stack_size = 0x1000;
static int _max_mem = 0x10000;
static int _mz_text_size = 0;
static int _mz_data_size = 0;
static int _mz_bss_size = 0;

static void write_mz_header(int init_ss, int init_sp, int file_size)
{
	int text_para = MZ_HEADER_PARAS;
	int bss_stack_paras = (_mz_bss_size + _stack_size + 15) / 16;

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

	for (int i = MZ_HEADER_SIZE; i < MZ_HEADER_PARAS * 16; i++)
		outb(0);
}

char *format_name(void)
{
	return "MS-DOS MZ EXE (Independent Segments)";
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
	printf("-stack [SIZE]    : Stack size in bytes (default: 4096)\n");
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
	if (!strcmp(argv[*argi], "-max-mem"))
	{
		(*argi)++;
		if (*argi < argc)
			_max_mem = toint(argv[*argi]);
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
		out_seek(MZ_HEADER_PARAS * 16, SEEK_SET);

	/* TEXT: starts at load module offset 0, file offset header_size */
	section_t *sec = section_find("text");
	sec->start_pos = 0;
	process_objs(step, REC_SECTION_TEXT);
	_mz_text_size = sec->size;

	/* DATA: separate segment, DS:0 is first DATA byte in memory.
	 * File bytes start after TEXT padding. Labels start at 0. */
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

	/* BSS: separate segment, starts at SS:0. Not in file. */
	sec = section_find("bss");
	sec->start_pos = 0;
	process_objs(step, REC_SECTION_BSS);
	_mz_bss_size = sec->size;

	{
		int text_paras = (_mz_text_size + 15) / 16;
		int data_paras = (_mz_data_size + 15) / 16;
		int data_seg_delta = text_paras;
		int bss_seg_delta = data_seg_delta + data_paras;
		int stack_top = _mz_bss_size + _stack_size;
		int init_ss = bss_seg_delta;
		int init_sp = stack_top;

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

		consts_set_global(_objs, "__data_seg_delta__");
		consts_set_global(_objs, "__bss_seg_delta__");
		consts_set(_objs, "__data_seg_delta__", data_seg_delta);
		consts_set(_objs, "__bss_seg_delta__", bss_seg_delta);
		consts_set(_objs, "__stack_size__", _stack_size);
		consts_set(_objs, "__stack_top__", stack_top);
	}

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
