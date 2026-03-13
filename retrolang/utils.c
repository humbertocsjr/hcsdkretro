#include "retrolang.h"

char* get_filename(char* filename)
{
    char* last = NULL;
    char* current = (char*)filename;
    
    while (*current != '\0') 
    {
        if (*current == '/') 
        {
            last = current + 1;
        }
        current++;
    }
    return last;
}