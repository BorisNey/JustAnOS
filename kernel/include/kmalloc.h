#pragma once

#include <stdint.h>
#include <stddef.h>
#include "memory.h"
#include "vga_bios_term.h"
#include "../../libc/include/util.h"

typedef struct block_header{
    uint32_t size;
    uint8_t free;
    struct block_header* next;
    struct block_header* prev;
} block_header_t;

void init_kmalloc(uint32_t initial_heap_size);
void change_heap_size(uint32_t new_size);
void* kmalloc(uint32_t size);
void kfree(void* ptr);