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
void *halloc(struct heap_t *h, size_t size)
{
    struct chunk_t *current;
    struct chunk_t *new_chunk;

    // Validate heap pointer and requested size
    if (h == NULL || size == 0) {
        errno = EINVAL; // Invalid arguments
        return NULL;
    }

    // If not enough available memory, fail early
    if (size > h->avail) {
        errno = ENOMEM; // Not enough memory in heap
        return NULL;
    }

    // Start traversing chunks from the beginning of the heap
    current = h->start;

    // First-fit search for a free chunk with sufficient size
    while (current != NULL) {

        // Check if this chunk is free and large enough
        if (!current->inuse && current->size >= size) {

            // Check if we can split the chunk
            // We need space for requested size + metadata of a new chunk
            if (current->size >= size + sizeof(struct chunk_t) + 1) {

                // Create a new chunk after the allocated space
                new_chunk = (struct chunk_t *)(
                    (uint8_t *)current + sizeof(struct chunk_t) + size
                );

                // Set the size of the remaining free chunk
                new_chunk->size = current->size - size - sizeof(struct chunk_t);

                // Mark the new chunk as free
                new_chunk->inuse = 0;

                // Link the new chunk to the list
                new_chunk->next = current->next;

                // Shrink the current chunk to requested size
                current->size = size;

                // Link current chunk to the new chunk
                current->next = new_chunk;
            }

            // Mark current chunk as in use
            current->inuse = 1;

            // Update available memory in heap
            h->avail -= current->size;

            // Return pointer to payload (skip metadata)
            return (void *)((uint8_t *)current + sizeof(struct chunk_t));
        }

        // Move to the next chunk in the list
        current = current->next;
    }

    // No suitable chunk found
    errno = ENOMEM;
    return NULL;
}

//frees dynamically allocated memory obtained by virtualHalloc
//ptr : pointer returned by halloc
void hfree(struct heap_t *h, void *ptr)
{
    struct chunk_t *chunk;
    struct chunk_t *current;

    // Validate input
    if (h == NULL || ptr == NULL) {
        errno = EINVAL;
        return;
    }

    // Retrieve chunk metadata from payload pointer
    chunk = (struct chunk_t *)((uint8_t *)ptr - sizeof(struct chunk_t));

    // Prevent double free
    if (!chunk->inuse) {
        errno = EINVAL;
        return;
    }

    // Mark the chunk as free
    chunk->inuse = 0;

    // Update available memory
    h->avail += chunk->size;

    // Start from heap beginning to find this chunk
    current = h->start;

    // Traverse chunks to find neighbors for merging
    while (current != NULL) {

        // Merge with next chunk if both are free
        if (!current->inuse &&
            current->next != NULL &&
            !current->next->inuse) {

            // Increase current chunk size by next chunk size + metadata
            current->size += sizeof(struct chunk_t) + current->next->size;

            // Remove next chunk from list
            current->next = current->next->next;

            // Continue checking in case of multiple free neighbors
            continue;
        }

        // Move to next chunk
        current = current->next;
    }
}

int main(void)
{
    struct heap_t heap = {0} ;           // Our single heap instance
    void *memory;                 // Raw memory buffer for the heap
    void *p1, *p2, *p3;            // Test allocation pointers
    uint32_t heap_size = 1024;     // Total heap size in bytes (1 KB)

    // Allocate raw memory for the heap using VirtualAlloc
    memory = VirtualAlloc(
        NULL,                      // Let the OS choose the address
        heap_size,                 // Total size of heap memory
        MEM_RESERVE | MEM_COMMIT,  // Reserve and commit memory
        PAGE_READWRITE             // Read/write access
    );

    // Check if raw memory allocation failed
    if (memory == NULL) {
        perror("VirtualAlloc failed");
        return 1;
    }

    // Initialize the heap structure on top of raw memory
    if (hinit(&heap, memory, heap_size) != 0) {
        perror("hinit failed");
        VirtualFree(memory, 0, MEM_RELEASE);
        return 1;
    }

    printf("Heap initialized successfully\n");
    printf("Available memory: %u bytes\n\n", heap.avail);

    // Allocate first block
    p1 = halloc(&heap, 100);
    if (p1 == NULL) {
        perror("halloc p1 failed");
    } else {
        printf("Allocated p1 (100 bytes)\n");
        printf("Available memory: %u bytes\n\n", heap.avail);
    }

    // Allocate second block
    p2 = halloc(&heap, 200);
    if (p2 == NULL) {
        perror("halloc p2 failed");
    } else {
        printf("Allocated p2 (200 bytes)\n");
        printf("Available memory: %u bytes\n\n", heap.avail);
    }

    // Allocate third block
    p3 = halloc(&heap, 50);
    if (p3 == NULL) {
        perror("halloc p3 failed");
    } else {
        printf("Allocated p3 (50 bytes)\n");
        printf("Available memory: %u bytes\n\n", heap.avail);
    }

    // Free the second block
    hfree(&heap, p2);
    printf("Freed p2 (200 bytes)\n");
    printf("Available memory: %u bytes\n\n", heap.avail);

    // Free the first block
    hfree(&heap, p1);
    printf("Freed p1 (100 bytes)\n");
    printf("Available memory: %u bytes\n\n", heap.avail);

    // Free the third block
    hfree(&heap, p3);
    printf("Freed p3 (50 bytes)\n");
    printf("Available memory: %u bytes\n\n", heap.avail);

    // Release the entire heap memory back to the OS
    VirtualFree(memory, 0, MEM_RELEASE);

    printf("Heap memory released. Test finished successfully.\n");

    return 0;
}