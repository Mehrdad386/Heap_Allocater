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

void hinit (struct heap_t* h){
    h->start = NULL ;
    h->avail = 0 ;
}

void *halloc (struct heap_t* h , size_t size){

}

void hfree (struct heap_t* h , void* ptr){

}

int main()
{
    return 0;
}