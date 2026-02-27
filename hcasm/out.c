#include "asm.h"

FILE *_out = NULL;
FILE *_dump = NULL;

void out_open(char *name, char *dump_name)
{
    _out = fopen(name, "wb");
    if(!_out) error("can't create file: %s", name);
    if(dump_name)
    {
        _dump = fopen(dump_name, "w");
        if(!_dump) error("can't create file: %s", dump_name);
    }
}

void out_close()
{
    fclose(_out);
}

void outb(int value)
{
    fwrite(&value, 1, 1, _out);
}

void outw(int value)
{
    fwrite(&value, 1, 2, _out);
}

void out(rectype_t type, int16_t value, uint16_t aux, void *data, uint8_t data_size)
{
    outb(type);
    outb(data_size);
    outw(value);
    outw(aux);
    for(int i = 0; i< data_size; i++)
    {
        outb(((char *)data)[i]);
    }
    if(_dump)
    {
        bool print_hex = false;
        fprintf(_dump, "$%02x: ", type);
        switch(type)
        {
            case REC_CPU_8080:
                fprintf(_dump, "CPU: Intel 8080 or Compatibles\n");
                break;
            case REC_CPU_8085:
                fprintf(_dump, "CPU: Intel 8085 or Compatibles\n");
                break;
            case REC_CPU_8086:
                fprintf(_dump, "CPU: Intel 8086 or Compatibles\n");
                break;
            case REC_CPU_Z80:
                fprintf(_dump, "CPU: Zilog Z80 or Compatibles\n");
                break;
            case REC_FILENAME:
                fprintf(_dump, "FILENAME: %s\n", (char*)data);
                break;
            case REC_END_OF_FILE:
                fprintf(_dump, "END OF FILE: %s\n", (char*)data);
                break;
            case REC_SECTION_TEXT:
                fprintf(_dump, "SECTION: TEXT\n");
                break;
            case REC_DATA:
                fprintf(_dump, "DATA:\n");
                print_hex = true;
                break;
            case REC_POSITION:
                fprintf(_dump, "POSITION: %i:%i\n", value, aux);
                break;
            case REC_CONST_LABEL:
                fprintf(_dump, "LABEL: %s\n", (char*)data);
                break;
            case REC_CONST_CUSTOM:
                fprintf(_dump, "CONSTANT: %s = %i\n", (char*)data, value);
                break;
            case REC_CONST_AS_GLOBAL_LABEL:
                fprintf(_dump, "EXPORT LABEL TO GLOBAL LABEL LIST: %s\n", (char*)data);
                break;
            case REC_EXPR_RESET:
                fprintf(_dump, "EXPR: RESET\n");
                break;
            case REC_EXPR_POP_REPEAT_TIMES:
                fprintf(_dump, "EXPR: POP; REPEAT XX TIMES\n");
                break;
            case REC_EXPR_REPEAT_TIMES_END:
                fprintf(_dump, "EXPR: END REPEAT\n");
                break;
            case REC_EXPR_ADD:
                fprintf(_dump, "EXPR: POP; POP; ADD; PUSH\n");
                break;
            case REC_EXPR_SUB:
                fprintf(_dump, "EXPR: POP; POP; SUB; PUSH\n");
                break;
            case REC_EXPR_MUL:
                fprintf(_dump, "EXPR: POP; POP; MUL; PUSH\n");
                break;
            case REC_EXPR_MOD:
                fprintf(_dump, "EXPR: POP; POP; MOD; PUSH\n");
                break;
            case REC_EXPR_DIV:
                fprintf(_dump, "EXPR: POP; POP; DIV; PUSH\n");
                break;
            case REC_EXPR_AND:
                fprintf(_dump, "EXPR: POP; POP; AND; PUSH\n");
                break;
            case REC_EXPR_OR:
                fprintf(_dump, "EXPR: POP; POP; OR; PUSH\n");
                break;
            case REC_EXPR_SHL:
                fprintf(_dump, "EXPR: POP; POP; SHL; PUSH\n");
                break;
            case REC_EXPR_PUSH_OFFSET:
                fprintf(_dump, "EXPR: PUSH CURRENT POSITION (MNEMONIC OFFSET: %i)\n", value);
                break;
            case REC_EXPR_PUSH_VALUE:
                fprintf(_dump, "EXPR: PUSH VALUE: 0x%04x (%i)\n", value, value);
                break;
            case REC_EXPR_PUSH_CONST:
                fprintf(_dump, "EXPR: PUSH CONST: %s\n", (char*)data);
                break;
            case REC_EXPR_POP_INT8_EMIT:
                fprintf(_dump, "EXPR: POP: EMIT 8BIT VALUE\n");
                break;
            case REC_EXPR_POP_INT16_EMIT:
                fprintf(_dump, "EXPR: POP: EMIT 16BIT VALUE\n");
                break;
            case REC_EXPR_POP_INT16_RELOCATABLE_EMIT:
                fprintf(_dump, "EXPR: POP: EMIT 16BIT RELOCATABLE VALUE\n");
                break;
            default:
                fprintf(_dump, "UNKNOWN TYPE: %i\n", type);
                print_hex = true;
                break;
        }
        if(print_hex)
        {
            for(int i = 0 ; i < data_size; i++)
            {
                if(!(i & 0xf) && i) fprintf(_dump, "\n");
                if(!(i & 0xf)) fprintf(_dump, "  0x%02x: ", i);
                fprintf(_dump, "%02x ", ((char*)data)[i] & 0xff);
            }
            fprintf(_dump, "\n");
        }
    }
}