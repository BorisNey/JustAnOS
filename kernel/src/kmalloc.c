#include "../include/kmalloc.h"

static uint32_t heap_start; // Heap start in physical address space
static uint32_t heap_size; // Heap size in physical address space

static block_header_t* heap_block_list_start = NULL;

void init_kmalloc(uint32_t initial_heap_size){
    heap_start = KERNEL_HEAP_START;
    heap_size = 0;

    change_heap_size(initial_heap_size);

    bios_term_print("DBG: Heap initialization success\n");
    return;
}

/*
* Changes the Heap Size in virtual and physical space
*/
void change_heap_size(uint32_t new_size){
    uint32_t old_page_top = CEIL_DIV(heap_size, PAGE_SIZE); // last usable old heap page
    uint32_t new_page_top = CEIL_DIV(new_size, PAGE_SIZE); // last usable new heap page
    uint32_t diff = new_page_top - old_page_top; // Difference between old an new needs to be mapped

    for (uint32_t i = 0; i < diff; i++){
        uint32_t phys_addr = pmm_alloc_page_frame();
        map_page(heap_start + old_page_top * PAGE_SIZE + i * PAGE_SIZE, phys_addr, PAGE_FLAG_WRITE);
    }

    heap_size = new_size;

    return;
}

/*
* Reserves a part of a heap of a specific size, retruns its address 
*/
void* kmalloc(uint32_t size) {
    block_header_t* curr = heap_block_list_start;
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
    uint32_t old_heap_size = heap_size;
    change_heap_size(heap_size + sizeof(block_header_t) + size);

    // Place new block at the end of the old heap
    block_header_t* new_block = (block_header_t*)(heap_start + old_heap_size);
    new_block->size = size;
    new_block->free = 0;
    new_block->next = NULL;

    // Link to the list
    if (heap_block_list_start == NULL) {
        heap_block_list_start = new_block;
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
        bios_term_print("ERROR: Nothing to free\n");
    }
    return;
}
