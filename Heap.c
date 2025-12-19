#include <stdio.h>
#include <stdint.h>
#include <errno.h>

struct chunk_t{
    uint32_t size;
    uint8_t inuse; // boolean
    struct chunk_t *next;
};

struct heap_t{
    struct chunk *start;
    uint32_t avail; // available memory
};

int hinit (struct heap_t* h , void *mem , uint32_t size){
    struct chunk_t *first ; //creating first node of our linked_list of chunks

    //validate input pointers
    //if h == NULL , we can't write heap metadata
    //if mem == NULL , there is no memory to build the heap on
    if(h== NULL || mem == NULL){
        errno = EFAULT ; //invalid memory access and bad addressing error
        return -1 ;
    }


}

void *halloc (struct heap_t* h , size_t size){

}

void hfree (struct heap_t* h , void* ptr){

}

int main()
{
    return 0;
}