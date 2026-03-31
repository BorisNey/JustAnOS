#include "../include/kmalloc.h"

static uint32_t heap_start; // Heap start in physical address space
static uint32_t heap_size; // Heap size in physical address space
static uint32_t threshhold; // TODO: Why?
uint8_t kmalloc_initialized = FALSE;

void init_kmalloc(uint32_t initial_heap_size){
    heap_start = KERNEL_HEAP_START;
    heap_size = 0;
    threshhold = 0;
    kmalloc_initialized = TRUE;

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
        map_page(KERNEL_HEAP_START + old_page_top * PAGE_SIZE + i * PAGE_SIZE, phys_addr, PAGE_FLAG_WRITE);
    }

    return;
}

void* kalloc(size_t size){
    /*
    pagesize: 4KB
    bitmap: 1 bit represents 1 page

    
    */
}
