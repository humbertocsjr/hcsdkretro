#include "link.h"

FILE *_out = NULL;

void out_open(char *name)
{
    _out = fopen(name, "wb");
    if(!_out) error("can't create file: %s", name);
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

void out(void *data, int data_size)
{
    if(data_size == 0) return;
    fwrite(data, 1, data_size, _out);
}