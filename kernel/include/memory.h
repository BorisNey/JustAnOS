#pragma once

#include <stddef.h>
#include <stdint.h>
#include "vga_bios_term.h"
#include "multiboot.h"
#include "../../libc/include/util.h"

#define KERNEL_START        0xC0000000
#define KERNEL_HEAP_START   0xD0000000

#define REC_PAGE_DIR        ((uint32_t*)0xFFFFF000)
#define REC_PAGE_TABLE(i)   ((uint32_t*)(0xFFC00000 + ((i)  << 12)))

#define PAGE_FLAG_PRESENT   (1 << 0)
#define PAGE_FLAG_WRITE     (1 << 1)
#define PAGE_FLAG_OWNER     (1 << 9)

#define NUM_PAGE_DIRS 256
/*
    0x10000000 = 4GB: entire 32-bit physical address space
    0x1000 = 4KB: page size
    4GB / 4KB = 1,048,576 possible page frames
*/
#define NUM_PAGE_FRAMES (0x100000000 / 0x1000)

extern uint32_t _kernel_end;
extern uint32_t initial_page_dir[1024];

void init_memory(multiboot_info_struct* boot_info);
void invalidate(uint32_t vaddr);
void pmm_init(uint32_t mem_low, uint32_t mem_high);
uint32_t pmm_alloc_page_frame();
void mem_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
uint32_t* mem_get_curr_page_dir();
void mem_change_curr_page_dir(uint32_t* page_dir);
void sync_page_dirs();
