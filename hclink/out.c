#include "link.h"

FILE *_out = NULL;

// [English] Open the output file for writing in binary mode
// [Portuguese] Abre o arquivo de saída para escrita em modo binário
void out_open(char *name)
{
    _out = fopen(name, "wb");
    if (!_out)
        error("can't create file: %s", name);
}

// [English] Close the output file
// [Portuguese] Fecha o arquivo de saída
void out_close()
{
    fclose(_out);
}

// [English] Write a single byte (8 bits) to the output file
// [Portuguese] Escreve um único byte (8 bits) no arquivo de saída
void outb(int value)
{
    fwrite(&value, 1, 1, _out);
}

// [English] Write a 16-bit word (little-endian) to the output file
// [Portuguese] Escreve uma palavra de 16 bits (little-endian) no arquivo de saída
void outw(int value)
{
    fwrite(&value, 1, 2, _out);
}

// [English] Write an arbitrary block of data to the output file
// [Portuguese] Escreve um bloco arbitrário de dados no arquivo de saída
void out(void *data, int data_size)
{
    if (data_size == 0)
        return;
    fwrite(data, 1, data_size, _out);
}

// [English] Seek to a position in the output file
// [Portuguese] Posiciona o cursor no arquivo de saída
void out_seek(long offset, int whence)
{
    fseek(_out, offset, whence);
}

// [English] Get the current position in the output file
// [Portuguese] Obtém a posição atual no arquivo de saída
long out_tell(void)
{
    return ftell(_out);
}
