#include <stdio.h>
#include <stdint.h>
#include <errno.h>

struct chunk_t{
    uint32_t size;
    uint8_t inuse; // boolean
    struct chunk_t *next;
};

struct heap_t{
    struct chunk_t *start;
    uint32_t avail; // available memory
};

//this function is for initializing heap
//notes : void* mem is a pointer to our memory with no type or limit (raw memory)
int hinit (struct heap_t* h , void *mem , uint32_t size){
    struct chunk_t *first ; //creating first node of our linked_list of chunks

    //validate input pointers
    //if h == NULL , we can't write heap metadata
    //if mem == NULL , there is no raw memory to build the heap on
    if(h== NULL || mem == NULL){
        errno = EFAULT ; //invalid memory access and bad addressing error
        return -1 ;
    }

    //prevent re-initializaation of an already initialized heap
    //if h.start != NUlL it means the heaap was initiaalized before and re-initialization will corrupt heap structure
    if(h->start != NULL){
        errno = EBUSY ; //the heap is a resource , and it is alreaady in use
        return -1 ;
    }

    //check for minimum required memory size
    //the heam must contain at least one chunk
    // size <= size of chunk means that we do not have even one chunk so error will occur
    if(size <= sizeof(struct chunk_t)){
        errno = ENOMEM ; //it indicates insufficient memory
        return -1 ;
    }


    first = (struct chunk_t *) mem ; //initializing memory to our first chunck (before mem we did a typecasting and we say it to compiler that mem will point at a chunk)
    first->size = size - sizeof(struct chunk_t) ; //initializing the chunk size (total minus metadata)
    first->inuse = 0 ; //mark the chunk as free for allocation (0 means free)
    first->next = NULL ; //since it is the only chunk at initialization


    h->start = first ; //attach the first chunk to heap structure
    h->avail = first->size ; // initialize available memory counter (this value decrease in allocation and increase in deaallocation)


}

void *halloc (struct heap_t* h , size_t size){

}

void hfree (struct heap_t* h , void* ptr){

}

int main()
{
    return 0;
}