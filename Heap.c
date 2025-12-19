#include <stdio.h>
#include <stdint.h>

struct chunk_t{
    uint32_t size;
    uint8_t inuse; // boolean
    struct chunk_t *next;
};

struct heap_t{
    struct chunk *start;
    uint32_t avail; // available memory
};


int main()
{
    return 0;
}