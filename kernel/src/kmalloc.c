#include "../include/kmalloc.h"

static uint32_t heap_start;
static uint32_t heap_size;
static uint32_t threshhold;
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

void change_heap_size(uint32_t new_size){
    uint32_t old_page_top = CEIL_DIV(heap_size, 0x1000);
    uint32_t new_page_top = CEIL_DIV(new_size, 0x1000);

    uint32_t diff = new_page_top - old_page_top;

    for (uint32_t i = 0; i < diff; i++){
        uint32_t phys = pmm_alloc_page_frame();
        mem_map_page(KERNEL_HEAP_START + old_page_top * 0x1000 + i *0x1000,
            phys, PAGE_FLAG_WRITE);
    }

    return;
}
