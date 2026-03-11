#include "../retrolang.h"


char *cpu_ext()
{
    return "8086";
}

char *cpu_name()
{
    return "8086/8088";
}

void cpu_init()
{
    // Default data types
    datatype_add("u8", NATIVETYPE_8BITS, false);
    datatype_add("s8", NATIVETYPE_8BITS, true);
    datatype_add("u16", NATIVETYPE_16BITS, false);
    datatype_add("s16", NATIVETYPE_16BITS, true);
    // Default aliases
    datatype_add("pointer", NATIVETYPE_16BITS, false);
    datatype_add("size", NATIVETYPE_16BITS, false);
    datatype_add("int", NATIVETYPE_16BITS, true);
    datatype_add("char", NATIVETYPE_8BITS, true);
}

