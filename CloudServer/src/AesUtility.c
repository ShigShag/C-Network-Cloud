#include <stdlib.h>
#include "../inc/AesUtility.h"

uint8_t *GetRandomBytes(uint32_t size)
{
    uint8_t* buffer = (uint8_t*) calloc(size, sizeof(uint8_t));
    if(buffer == NULL) return NULL;

    for (int i = 0; i < size; i++)
    {
        buffer[i] = rand();
    }
    return buffer;
}