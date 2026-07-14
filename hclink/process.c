#include "link.h"

static char *_filename = NULL;
static int _line = 0;
static int _column = 0;
static uint16_t _file_idx = 0;
static section_t *_curr_section = NULL;

int (*format_adjust_value)(int value, int position, rectype_t section) = NULL;
void (*format_record_reloc)(int position, rectype_t section) = NULL;

// [English] Print a formatted error message with source location (file:line:column)
// [Portuguese] Imprime uma mensagem de erro formatada com localização da fonte (arquivo:linha:coluna)
void process_error(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (_filename && _line && _column)
        fprintf(stderr, "%s:%i:%i: ", _filename, _line, _column);
    else if (_filename && _line)
        fprintf(stderr, "%s:%i: ", _filename, _line);
    else if (_filename)
        fprintf(stderr, "%s: ", _filename);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}

// [English] Process a single object file: read records and execute them for a given section and step
// [Portuguese] Processa um único arquivo objeto: lê registros e os executa para uma dada seção e etapa
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
    _file_idx = debug_add_file(_filename);
    _line = 0;
    _column = 0;
    // [English] Main record processing loop
    // [Portuguese] Loop principal de processamento de registros
    while (obj_read(obj, &rec))
    {
        switch ((rectype_t)rec.header.type)
        {
        // [English] Section: Handle section type selection
        // [Portuguese] Seção: Gerencia seleção de tipo de seção
        case REC_SECTION_TEXT:
            _curr_section = section_find("text");
            break;
        case REC_SECTION_DATA:
            _curr_section = section_find("data");
            break;
        case REC_SECTION_BSS:
            _curr_section = section_find("bss");
            break;

        // [English] Section: Expression stack management (reset and push operations)
        // [Portuguese] Seção: Gerenciamento da pilha de expressão (reset e operações push)
        case REC_EXPR_RESET:
            stack_reset();
            break;
        case REC_EXPR_PUSH_VALUE:
            stack_push(rec.header.value);
            break;
        case REC_EXPR_PUSH_VALUE_UNSIGNED:
            stack_push(*(uint16_t *)&rec.header.value);
            break;
        case REC_EXPR_PUSH_OFFSET:
            stack_push(_curr_section->position + rec.header.value);
            break;
        case REC_EXPR_PUSH_CONST:
            if (!strcmp((char *)rec.data, "_ebss"))
            {
                stack_push(consts_get(obj, "__bss_end__"));
            }
            else if (!strcmp((char *)rec.data, "_edata"))
            {
                stack_push(consts_get(obj, "__data_end__"));
            }
            else if (!strcmp((char *)rec.data, "_etext"))
            {
                stack_push(consts_get(obj, "__text_end__"));
            }
            else
            {
                val1 = consts_get(obj, (char *)rec.data);
                const_is_offset = consts_is_offset(obj, (char *)rec.data);
                stack_push(val1);
                if (step == STEP_GENERATE && !consts_exists(obj, (char *)rec.data))
                {
                    process_error("constant not found: %s", (char *)rec.data);
                }
            }
            break;
        case REC_EXPR_PUSH_SEGMENT:
            {
                val1 = consts_get(obj, (char *)rec.data);
                const_is_offset = consts_is_offset(obj, (char *)rec.data);
                stack_push(val1);
                if (step == STEP_GENERATE && !consts_exists(obj, (char *)rec.data))
                {
                    process_error("constant not found: %s", (char *)rec.data);
                }
            }
            break;

        // [English] Section: Binary arithmetic and bitwise operations
        // [Portuguese] Seção: Operações aritméticas binárias e bitwise
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

        // [English] Section: Pop operations - store constants or emit bytes/words to output
        // [Portuguese] Seção: Operações pop - armazenar constantes ou emitir bytes/words na saída
        case REC_EXPR_POP_TO_CONST:
            val1 = stack_pop();
            consts_set(obj, (char *)rec.data, val1);
            const_is_offset = false;
            break;
        case REC_EXPR_POP_INT8_EMIT:
            val1 = stack_pop();
            if (step == STEP_GENERATE && _curr_section->section == section)
            {
                if (section == REC_SECTION_BSS)
                    process_error("invalid bss content.");
                outb(val1);
            }
            if (step == STEP_GENERATE && (val1 < INT8_MIN || val1 > UINT8_MAX))
            {
                process_error("byte value overflow: %i", val1);
            }
            _curr_section->position += 1;
            _curr_section->size += 1;
            const_is_offset = false;
            break;
        case REC_EXPR_POP_INT16_EMIT:
            val1 = stack_pop();
            if (step == STEP_GENERATE && _curr_section->section == section)
            {
                if (section == REC_SECTION_BSS)
                    process_error("invalid bss content.");
                if (format_adjust_value)
                    val1 = format_adjust_value(val1, _curr_section->position, section);
                outw(val1);
            }
            _curr_section->position += 2;
            _curr_section->size += 2;
            const_is_offset = false;
            break;
        case REC_EXPR_POP_INT16_RELOCATABLE_EMIT:
            val1 = stack_pop();
            if (step == STEP_GENERATE && _curr_section->section == section)
            {
                if (section == REC_SECTION_BSS)
                    process_error("invalid bss content.");
                if (format_adjust_value)
                    val1 = format_adjust_value(val1, _curr_section->position, section);
                outw(val1);
                if (format_record_reloc)
                    format_record_reloc(_curr_section->position, section);
            }
            if (step == STEP_GENERATE && section == REC_SECTION_RELOC)
            {
                if (const_is_offset)
                    outw(_curr_section->position);
            }
            _curr_section->position += 2;
            _curr_section->size += 2;
            const_is_offset = false;
            break;

        // [English] Section: Repeat block (loop) handling
        // [Portuguese] Seção: Gerenciamento de bloco de repetição (loop)
        case REC_EXPR_POP_REPEAT_TIMES:
            repeat_pos = obj->next_rec;
            repeat_count = stack_pop();
            const_is_offset = false;
            break;
        case REC_EXPR_REPEAT_TIMES_END:
            if (--repeat_count > 0)
                fsetpos(obj->file, &repeat_pos);
            break;

        // [English] Section: Constant and label definitions
        // [Portuguese] Seção: Definições de constantes e rótulos
        case REC_CONST_LABEL:
            consts_set(obj, (char *)rec.data, _curr_section->position);
            consts_set_offset(obj, (char *)rec.data);
            consts_set_section(obj, (char *)rec.data, _curr_section->section);
            break;
        case REC_CONST_CUSTOM:
            consts_set(obj, (char *)rec.data, rec.header.value);
            break;
        case REC_CONST_AS_GLOBAL_LABEL:
            consts_set_global(obj, (char *)rec.data);
            break;

        // [English] Section: CPU type detection records
        // [Portuguese] Seção: Registros de detecção de tipo de CPU
        case REC_CPU_8080:
        case REC_CPU_8085:
        case REC_CPU_8086:
        case REC_CPU_Z80:
            _cpu = rec.header.type;
            break;

        // [English] Section: File and position metadata records
        // [Portuguese] Seção: Registros de metadados de arquivo e posição
        case REC_FILENAME:
            break;
        case REC_POSITION:
            _line = rec.header.value;
            _column = rec.header.aux;
            break;

        // [English] Section: Data emission and reservation
        // [Portuguese] Seção: Emissão e reserva de dados
        case REC_DATA:
            if (step == STEP_GENERATE && _curr_section->section == section)
            {
                if (section == REC_SECTION_BSS)
                    process_error("invalid bss content.");
                out(rec.data, rec.header.data_size);
            }
            // [English] Collect debug info: map line to address
            // [Portuguese] Coleta info de debug: mapeia linha para endereço
            if (step == STEP_PROCESS_FILES && _line > 0)
            {
                debug_add_line(_curr_section->position, _file_idx, _line, _column);
                _line = 0;  // Reset to avoid duplicate mappings
                _column = 0;
            }
            _curr_section->position += rec.header.data_size;
            _curr_section->size += rec.header.data_size;
            break;
        case REC_DATA_RESERVE:
            if (step == STEP_GENERATE && _curr_section->section == section && section != REC_SECTION_BSS)
            {
                for (int i = 0; i < rec.header.value; i++)
                {
                    outb(0);
                }
            }
            _curr_section->position += rec.header.value;
            _curr_section->size += rec.header.value;
            break;

        // [English] Section: Unknown record type - report error
        // [Portuguese] Seção: Tipo de registro desconhecido - reporta erro
        default:
            process_error("NOT IMPLEMENTED RECORD TYPE: 0x%X (%i)", rec.header.type, rec.header.type);
            break;
        }
    }
}

// [English] Process all object files for a given section type, returning final position
// [Portuguese] Processa todos os arquivos objeto para um dado tipo de seção, retornando a posição final
size_t process_objs(step_t step, rectype_t section)
{
    object_file_t *obj = _objs;
    section_t *curr_section;
    // [English] Register global constants for section boundaries
    // [Portuguese] Registra constantes globais para limites de seção
    consts_set_global(_objs, "__text_start__");
    consts_set_global(_objs, "__data_start__");
    consts_set_global(_objs, "__bss_start__");
    consts_set_offset(_objs, "__text_start__");
    consts_set_offset(_objs, "__data_start__");
    consts_set_offset(_objs, "__bss_start__");
    consts_set_global(_objs, "__text_end__");
    consts_set_global(_objs, "__data_end__");
    consts_set_global(_objs, "__bss_end__");
    consts_set_offset(_objs, "__text_end__");
    consts_set_offset(_objs, "__data_end__");
    consts_set_offset(_objs, "__bss_end__");
    consts_set_global(_objs, "__text_size__");
    consts_set_global(_objs, "__data_size__");
    consts_set_global(_objs, "__bss_size__");
    consts_set_global(_objs, "_end");
    consts_set_global(_objs, "_etext");
    consts_set_global(_objs, "_edata");
    consts_set_global(_objs, "_ebss");
    consts_set_offset(_objs, "_end");
    consts_set_offset(_objs, "_etext");
    consts_set_offset(_objs, "_edata");
    consts_set_offset(_objs, "_ebss");
    consts_set_section(_objs, "__text_start__", REC_SECTION_TEXT);
    consts_set_section(_objs, "__data_start__", REC_SECTION_DATA);
    consts_set_section(_objs, "__bss_start__", REC_SECTION_DATA);
    consts_set_section(_objs, "__text_end__", REC_SECTION_TEXT);
    consts_set_section(_objs, "__data_end__", REC_SECTION_DATA);
    consts_set_section(_objs, "__bss_end__", REC_SECTION_DATA);
    consts_set_section(_objs, "_etext", REC_SECTION_TEXT);
    consts_set_section(_objs, "_edata", REC_SECTION_DATA);
    consts_set_section(_objs, "_ebss", REC_SECTION_DATA);
    // [English] Select the current section based on type
    // [Portuguese] Seleciona a seção atual baseada no tipo
    switch (section)
    {
    case REC_SECTION_TEXT:
        curr_section = section_find("text");
        break;
    case REC_SECTION_DATA:
        curr_section = section_find("data");
        break;
    case REC_SECTION_BSS:
        curr_section = section_find("bss");
        break;
    case REC_SECTION_RELOC:
        curr_section = section_find("reloc");
        break;
    default:
        curr_section = section_find("text");
        break;
    }
    section_reset_sizes();
    // [English] Process each object file in the linked list
    // [Portuguese] Processa cada arquivo objeto na lista encadeada
    while (obj)
    {
        process_obj(step, section, obj);
        obj = obj->next;
    }
    return curr_section->position;
}

// [English] Mark referenced object files as used in the link (dependency tracking)
// [Portuguese] Marca arquivos objeto referenciados como usados no link (rastreamento de dependências)
void process_filter(object_file_t *obj)
{
    record_t rec;
    object_file_t *ref;
    obj_reset(obj);
    obj->use_in_link = true;
    // [English] Scan all records for constant references to other objects
    // [Portuguese] Examina todos os registros em busca de referências de constantes a outros objetos
    while (obj_read(obj, &rec))
    {
        switch (rec.header.type)
        {
        case REC_EXPR_PUSH_CONST:
        case REC_EXPR_PUSH_SEGMENT:
        case REC_EXPR_POP_TO_CONST:
            ref = consts_get_obj(obj, (char *)rec.data);
            if (ref != NULL && ref != obj)
            {
                verbose("Reference: %s [%s -> %s]\n", (char *)rec.data, obj->name, ref->name);
                // [English] Recursively mark the referenced object
                // [Portuguese] Marca recursivamente o objeto referenciado
                if (!ref->use_in_link)
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

// [English] Find and keep the object file containing the _start label
// [Portuguese] Encontra e mantém o arquivo objeto que contém o rótulo _start
static void process_keep_start(void)
{
    object_file_t *obj = _objs;
    while (obj)
    {
        record_t rec;
        obj_reset(obj);
        // [English] Search for the _start global label in each object
        // [Portuguese] Procura pelo rótulo global _start em cada objeto
        while (obj_read(obj, &rec))
        {
            if (rec.header.type == REC_CONST_AS_GLOBAL_LABEL)
            {
                if (!strcmp((char *)rec.data, "_start"))
                {
                    obj->use_in_link = true;
                    verbose("Keep _start Object File: %s\n", obj->name);
                    return;
                }
            }
        }
        obj = obj->next;
    }
}

// [English] Remove object files that were filtered out (not referenced) from the linked list
// [Portuguese] Remove arquivos objeto que foram filtrados (não referenciados) da lista encadeada
void process_remove_filtered(object_file_t *obj)
{
    if (obj && obj->next)
    {
        if (!obj->next->use_in_link)
        {
            verbose("Remove unused Object File: %s\n", obj->next->name);
            obj->next = obj->next->next;
        }
        if (obj->next)
            process_remove_filtered(obj->next);
    }
}

// [English] Main processing entry point: iterates until constants stabilize or filter completes
// [Portuguese] Ponto de entrada principal de processamento: itera até constantes estabilizarem ou filtro completar
void process(step_t step)
{
    _curr_section = section_find("text");
    int tries = 0;
    do
    {
        consts_reset_changed();
        if (step == STEP_FILTER)
        {
            // [English] Filter phase: mark referenced objects and remove unused ones
            // [Portuguese] Fase de filtro: marca objetos referenciados e remove os não usados
            process_filter(_objs);
            process_keep_start();
            process_remove_filtered(_objs);
        }
        else
            format_process(step);
        tries++;
        if (tries > 1000)
        {
            error_consts_has_changed();
        }
    } while (step == STEP_PROCESS_FILES && consts_is_changed());
}

// [English] Reorder object list so the file containing _start is first (for correct initialization)
// [Portuguese] Reordena a lista de objetos para que o arquivo contendo _start seja o primeiro
void reorder_start_first(void)
{
    object_file_t *obj = _objs;
    object_file_t *found = NULL;
    char *found_name = NULL;

    // [English] Search for the object containing the _start label
    // [Portuguese] Procura pelo objeto contendo o rótulo _start
    while (obj)
    {
        record_t rec;
        obj_reset(obj);
        while (obj_read(obj, &rec))
        {
            if (rec.header.type == REC_CONST_AS_GLOBAL_LABEL)
            {
                if (!strcmp((char *)rec.data, "_start"))
                {
                    found = obj;
                    found_name = (char *)rec.data;
                    break;
                }
            }
        }
        if (found)
            break;
        obj = obj->next;
    }

    // [English] If not found or already first, nothing to do
    // [Portuguese] Se não encontrado ou já é o primeiro, nada a fazer
    if (!found || found == _objs)
        return;

    // [English] Unlink found object and move it to the head of the list
    // [Portuguese] Desvincula o objeto encontrado e o move para o início da lista
    object_file_t *prev = _objs;
    while (prev && prev->next != found)
        prev = prev->next;

    if (prev)
    {
        prev->next = found->next;
        found->next = _objs;
        _objs = found;
        verbose("Reorder: object '%s' moved to first (has %s)\n", found->name, found_name);
    }
}
