#include "kmalloc.h"

/*
IMPROVEMENTS:
    - give an error if there is no more space to allocate

*/

static uint32_t g_heap_start; // Heap start in physical address space
static uint32_t g_heap_size; // Heap size in physical address space
static uint8_t g_heap_init;

static block_header_t* g_heap_block_ls = NULL;

void initKmalloc(uint32_t initial_heap_size){
    g_heap_start = KERNEL_HEAP_START;
    g_heap_size = 0;

    if (increaseHeapSize(initial_heap_size) == 1){
        g_heap_init = 0;
        biosTermPrintf("ERR: Heap not initialized\n");
        return;
    }

    g_heap_init = 1;
    biosTermPrintf("DBG: Heap init success\n");
    return;
}

/*
* Changes the Heap Size in virtual and physical space
*/
int increaseHeapSize(uint32_t new_size){
    if (new_size < g_heap_size){
        biosTermPrintf("ERR: New heap size cannot be smaller than old heap size\n");
        return ERROR;
    }

    uint32_t old_page_top = CEIL_DIV(g_heap_size, PAGE_SIZE); // last usable old heap page
    uint32_t new_page_top = CEIL_DIV(new_size, PAGE_SIZE); // last usable new heap page
    uint32_t diff = new_page_top - old_page_top; // Difference between old an new needs to be mapped

    for (uint32_t i = 0; i < diff; i++){
        uint32_t phys_addr = allocPageFrame();
        mapAddr(g_heap_start + old_page_top * PAGE_SIZE + i * PAGE_SIZE, phys_addr, PAGE_FLAG_WRITE);
    }

    g_heap_size = new_size;

    return SUCCESS;
}

/*
* Reserves a part of a heap of a specific size, retruns its address 
*/
void* kmalloc(uint32_t size) {
    block_header_t* curr = g_heap_block_ls;
    block_header_t* last = NULL;
    while (curr != NULL){
        if (curr->size >= size && curr->free) {
            // if leftover is big enough for another block, split the current one
            if (curr->size >= size + sizeof(block_header_t) + (uint32_t)SPLIT_THRESHHOLD) {
                // create a new split block
                block_header_t* split_block = (block_header_t*)((uint8_t*)(curr + 1) + size);
                split_block->size = curr->size - size - sizeof(block_header_t);
                split_block->free = 1;
                split_block->next = curr->next;
                split_block->prev = curr;

                curr->next = split_block;
                curr->size = size;
            }
            curr->free = 0;
            return (void*)(curr + 1);  // pointer just after the header struct
        }
        last = curr;
        curr = curr->next;
    }

    // No free block found —> grow the heap and allign page
    uint32_t old_heap_size = g_heap_size;
    if (increaseHeapSize(g_heap_size + sizeof(block_header_t) + size) == ERROR){
        return NULL;
    }

    // Place new block at the end of the old heap
    block_header_t* new_block = (block_header_t*)(g_heap_start + old_heap_size);
    new_block->size = size;
    new_block->free = 0;
    new_block->next = NULL;

    // Link to the list
    if (g_heap_block_ls == NULL) {
        g_heap_block_ls = new_block;
        new_block->prev = NULL;
    }
    else{
        last->next = new_block;
        new_block->prev = last;
    }

    return (void*)(new_block + 1);
}

/*
* Sets the Block of the heap to free and merges it with other surrounding free blocks
*/
void kfree(void* ptr){
    if (ptr != NULL && !(((block_header_t*)ptr - 1)->free) ){
        block_header_t* block = (block_header_t*)ptr - 1;
        block->free = 1;

        // Merge with next block if also free
        if (block->next != NULL && block->next->free) {
            block->size += sizeof(block_header_t) + block->next->size;
            block->next = block->next->next;
        }

        // Merge with previous block if also free
        if (block->prev != NULL && block->prev->free) {
            block->prev->size += sizeof(block_header_t) + block->size;
            block->prev->next = block->next;
        }

    }
    else{
        biosTermPrintf("ERROR: Nothing to free\n");
    }
    return;
}
