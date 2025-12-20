#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include<windows.h>

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
int hinit (struct heap_t *h , void *mem , uint32_t size){
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

    return 0 ;
}

//to allocate size(name of varible) bytes from memory and return a pointer to the allocated space
void *halloc (struct heap_t *h , size_t size){

    struct chunk_t *chunk ;
    size_t total_size ;
    void *mem ;
    
    //the size of zero is not a valid allocation request
    if(size ==0){
        errno = EINVAL ; //invalid request
        return NULL ;
    }

    total_size = size + sizeof(struct chunk_t) ; //total memory required

    //we alloc memory using virtual alloc insteaad of mmap because we're on windows and have no access to mmap
    mem = VirtualAlloc(
        NULL, //IP address = null it means let the system choose the address
        total_size, //dwSize = total_size
        MEM_RESERVE | MEM_COMMIT, // f allocaation type : MEM_RESERVED : reserve the address space , MEM_COMMIT : allocates physical pages
        PAGE_READWRITE //flProtect : PAGE_READWRITE : memory is readable and writable
    );

    //check for allocation failure
    //virtualAlloc returns NULL on failure
    if(mem == NULL){
        errno = ENOMEM ; // not enough memory
        return NULL ;
    }

    //Treat the begginning of allocated memory as chunk metadata
    chunk = (struct chunk_t *) mem ; //type casting

    //initialization metadata fields
    chunk->size = size ; //number of bytes requested by user
    chunk->inuse = 1 ; //it is in use
    chunk->next = NULL ; //this chunk is standalone for now

    //return pointer to payload area , hiding metadata from user
    //we add size of chunk to the base address to skip metadata
    return (void *)((uint8_t *)mem + sizeof(struct chunk_t)) ;

}

//frees dynamically allocated memory obtained by virtualHalloc
//ptr : pointer returned by halloc
void hfree (struct heap_t *h ,void* ptr){
    struct chunk_t *chunk ;

    //check for null pointer
    if(ptr == NULL){
        return;
    }

    //calculate the address of chunck metadataa
    //user pointer points to payload so we subtract size of chunck from it to get metas
    chunk = (struct chunk_t *)((uint8_t *)ptr - sizeof(struct chunk_t)) ;

    //ensure the chunk was actually allocated
    if (!chunk->inuse){
        errno = EINVAL ; //invalid
        return;
    }

    chunk->inuse = 0 ; //mark chunk as free

    //release memory back to the OS


    //parameters :
    //IP address
    //dwSize : free the entire region allocated by virtual aalloc
    // dwFreeType : MEM_RELEASE : release the allocation
    //returns 0 on failure and non zero on success
    if(!VirtualFree(chunk , 0 , MEM_RELEASE)){
        errno = EFAULT ; //could not release memory
    }
}

int main()
{
    return 0;
}