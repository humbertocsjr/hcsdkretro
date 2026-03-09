#include "link.h"

static char *_filename = NULL;
static int _line = 0;
static int _column = 0;
static section_t *_curr_section = NULL;

void process_error(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if(_filename && _line && _column) fprintf(stderr, "%s:%i:%i: ", _filename, _line, _column);
    else if(_filename && _line) fprintf(stderr, "%s:%i: ", _filename, _line);
    else if(_filename) fprintf(stderr, "%s: ", _filename);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}


void process_obj(step_t step, rectype_t section, object_file_t *obj)
{
    record_t rec;
    fpos_t repeat_pos;
    int repeat_count;
    bool const_is_offset = false;
    _curr_section = section_find("text");
    int val1;
    int val2;
    obj_reset(obj);
    stack_reset();
    _filename = obj->name;
    _line = 0;
    _column = 0;
    while(obj_read(obj, &rec))
    {
        switch ((rectype_t)rec.header.type)
        {
            case REC_SECTION_TEXT:
                _curr_section = section_find("text");
                break;
            case REC_SECTION_DATA:
                _curr_section = section_find("data");
                break;
            case REC_SECTION_BSS:
                _curr_section = section_find("bss");
                break;
            case REC_EXPR_RESET:
                stack_reset();
                break;
            case REC_EXPR_PUSH_VALUE:
                stack_push(rec.header.value);
                break;
            case REC_EXPR_PUSH_VALUE_UNSIGNED:
                stack_push(*(uint16_t*)&rec.header.value);
                break;
            case REC_EXPR_PUSH_OFFSET:
                stack_push(_curr_section->position + rec.header.value);
                break;
            case REC_EXPR_PUSH_CONST:
                if(!strcmp((char*)rec.data, "_ebss"))
                {
                    stack_push(consts_get(obj, "__bss_end__"));
                }
                else if(!strcmp((char*)rec.data, "_edata"))
                {
                    stack_push(consts_get(obj, "__data_end__"));
                }
                else if(!strcmp((char*)rec.data, "_etext"))
                {
                    stack_push(consts_get(obj, "__text_end__"));
                }
                else
                {
                    val1 = consts_get(obj, (char *)rec.data);
                    const_is_offset = consts_is_offset(obj, (char*)rec.data);
                    stack_push(val1);
                    if(step == STEP_GENERATE && !consts_exists(obj, (char *)rec.data))
                    {
                        process_error("constant not found: %s", (char *)rec.data);
                    }
                }
                break;
            case REC_EXPR_ADD:
                val2 = stack_pop();
                val1 = stack_pop();
                stack_push(val1 + val2);
                break;
            case REC_EXPR_AND:
                val2 = stack_pop();
                val1 = stack_pop();
                stack_push(val1 & val2);
                break;
            case REC_EXPR_DIV:
                val2 = stack_pop();
                val1 = stack_pop();
                stack_push(val1 / val2);
                break;
            case REC_EXPR_MOD:
                val2 = stack_pop();
                val1 = stack_pop();
                stack_push(val1 % val2);
                break;
            case REC_EXPR_MUL:
                val2 = stack_pop();
                val1 = stack_pop();
                stack_push(val1 * val2);
                break;
            case REC_EXPR_NOT:
                val1 = stack_pop();
                stack_push(!val1);
                break;
            case REC_EXPR_OR:
                val2 = stack_pop();
                val1 = stack_pop();
                stack_push(val1 | val2);
                break;
            case REC_EXPR_XOR:
                val2 = stack_pop();
                val1 = stack_pop();
                stack_push(val1 ^ val2);
                break;
            case REC_EXPR_SUB:
                val2 = stack_pop();
                val1 = stack_pop();
                stack_push(val1 - val2);
                break;
            case REC_EXPR_SHR:
                val2 = stack_pop();
                val1 = stack_pop();
                stack_push(val1 >> val2);
                break;
            case REC_EXPR_SHL:
                val2 = stack_pop();
                val1 = stack_pop();
                stack_push(val1 << val2);
                break;
            case REC_EXPR_POP_TO_CONST:
                val1 = stack_pop();
                consts_set(obj, (char*)rec.data, val1);
                const_is_offset = false;
                break;
            case REC_EXPR_POP_INT8_EMIT:
                val1 = stack_pop();
                if(step == STEP_GENERATE && _curr_section->section == section)
                {
                    if(section == REC_SECTION_BSS) process_error("invalid bss content.");
                    outb(val1);
                }
                if(step == STEP_GENERATE && (val1 < INT8_MIN || val1 > UINT8_MAX))
                {
                    process_error("byte value overflow: %i", val1);
                }
                _curr_section->position += 1;
                _curr_section->size += 1;
                const_is_offset = false;
                break;
            case REC_EXPR_POP_INT16_EMIT:
                val1 = stack_pop();
                if(step == STEP_GENERATE && _curr_section->section == section)
                {
                    if(section == REC_SECTION_BSS) process_error("invalid bss content.");
                    outw(val1);
                }
                _curr_section->position += 2;
                _curr_section->size += 2;
                const_is_offset = false;
                break;
            case REC_EXPR_POP_INT16_RELOCATABLE_EMIT:
                val1 = stack_pop();
                if(step == STEP_GENERATE && _curr_section->section == section)
                {
                    if(section == REC_SECTION_BSS) process_error("invalid bss content.");
                    outw(val1);
                }
                if(step == STEP_GENERATE && section == REC_SECTION_RELOC)
                {
                    if(const_is_offset) outw(_curr_section->position);
                }
                _curr_section->position += 2;
                _curr_section->size += 2;
                const_is_offset = false;
                break;
            case REC_EXPR_POP_REPEAT_TIMES:
                repeat_pos = obj->next_rec;
                repeat_count = stack_pop();
                const_is_offset = false;
                break;
            case REC_EXPR_REPEAT_TIMES_END:
                if(--repeat_count > 0) fsetpos(obj->file, &repeat_pos);
                break;
            case REC_CONST_LABEL:
                consts_set(obj, (char*)rec.data, _curr_section->position);
                consts_set_offset(obj, (char*)rec.data);
                break;
            case REC_CONST_CUSTOM:
                consts_set(obj, (char*)rec.data, rec.header.value);
                break;
            case REC_CONST_AS_GLOBAL_LABEL:
                consts_set_global(obj, (char*)rec.data);
                break;
            case REC_CPU_8080:
            case REC_CPU_8085:
            case REC_CPU_8086:
            case REC_CPU_Z80:
                _cpu = rec.header.type;
                break;
            case REC_FILENAME:
                break;
            case REC_POSITION:
                _line = rec.header.value;
                _column = rec.header.aux;
                break;
            case REC_DATA:
                if(step == STEP_GENERATE && _curr_section->section == section)
                {
                    if(section == REC_SECTION_BSS) process_error("invalid bss content.");
                    out(rec.data, rec.header.data_size);
                }
                _curr_section->position += rec.header.data_size;
                _curr_section->size += rec.header.data_size;
                break;
            case REC_DATA_RESERVE:
                if(step == STEP_GENERATE && _curr_section->section == section && section != REC_SECTION_BSS)
                {
                    for(int i = 0; i < rec.header.value; i++)
                    {
                        outb(0);
                    }
                }
                _curr_section->position += rec.header.value;
                _curr_section->size += rec.header.value;
                break;
            default:
                process_error("NOT IMPLEMENTED RECORD TYPE: 0x%X (%i)", rec.header.type, rec.header.type);
                break;
        }
    }
}

size_t process_objs(step_t step, rectype_t section)
{
    object_file_t *obj = _objs;
    section_t *curr_section;
    consts_set_global(_objs, "__text_start__");
    consts_set_global(_objs, "__data_start__");
    consts_set_global(_objs, "__bss_start__");
    consts_set_global(_objs, "__text_end__");
    consts_set_global(_objs, "__data_end__");
    consts_set_global(_objs, "__bss_end__");
    consts_set_global(_objs, "__text_size__");
    consts_set_global(_objs, "__data_size__");
    consts_set_global(_objs, "__bss_size__");
    consts_set_global(_objs, "_end");
    consts_set_global(_objs, "_etext");
    consts_set_global(_objs, "_edata");
    consts_set_global(_objs, "_ebss");
    switch (section)
    {
        case REC_SECTION_TEXT:
            curr_section =  section_find("text");
            break;
        case REC_SECTION_DATA:
            curr_section =  section_find("data");
            break;
        case REC_SECTION_BSS:
            curr_section =  section_find("bss");
            break;
        case REC_SECTION_RELOC:
            curr_section =  section_find("reloc");
            break;
        default:
            curr_section =  section_find("text");
            break;
    }
    section_reset_sizes();
    while(obj)
    {
        process_obj(step, section, obj); 
        obj = obj->next;
    }
    return curr_section->position;
}

void process_filter(object_file_t *obj)
{
    record_t rec;
    object_file_t *ref;
    obj_reset(obj);
    obj->use_in_link = true;
    while(obj_read(obj, &rec))
    {
        switch (rec.header.type)
        {
            case REC_EXPR_PUSH_CONST:
            case REC_EXPR_POP_TO_CONST:
                ref = consts_get_obj(obj, (char*)rec.data);
                if(ref != NULL && ref != obj)
                {
                    verbose("Reference: %s [%s -> %s]\n", (char*)rec.data, obj->name, ref->name);
                    if(!ref->use_in_link)
                    {
                        process_filter(ref);
                    }
                }
                break;
            default:
                break;
        }
    }
}

void process_remove_filtered(object_file_t *obj)
{
    if(obj && obj->next)
    {
        if(!obj->next->use_in_link)
        {
            verbose("Remove unused Object File: %s\n", obj->next->name);
            obj->next = obj->next->next;
        }
        if(obj->next) process_remove_filtered(obj->next);
    }
}

void process(step_t step)
{
    _curr_section = section_find("text");
    int tries = 0;
    do
    {
        consts_reset_changed();
        if(step == STEP_FILTER)
        {
            process_filter(_objs);
            process_remove_filtered(_objs);
        }
        else format_process(step);
        tries++;
        if(tries > 1000)
        {
            error("fail to process files, labels keep changing.");
        }
    } while(step == STEP_PROCESS_FILES && consts_is_changed());
}

