#pragma once

#include <stdint.h>
#include <stddef.h>
#include "memory.h"
#include "vga_bios_term.h"
#include "../../libc/include/util.h"

#define SPLIT_THRESHHOLD 16 // For kmalloc to decide, if the data block is big enough to split

typedef struct block_header{
    uint32_t size;
    uint8_t free;
    struct block_header* next;
    struct block_header* prev;
} block_header_t;

void initKmalloc(uint32_t initial_heap_size);
void changeHeapSize(uint32_t new_size);
void* kmalloc(uint32_t size);
void kfree(void* ptr);