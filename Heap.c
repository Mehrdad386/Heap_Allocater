#include <stdio.h>
#include <stdint.h>

struct chunk_t{
    uint32_t size;
    uint8_t inuse; // boolean
    struct chunk_t *next;
};

int main()
{
    return 0;
}