#pragma once

#include <stddef.h>
#include <stdint.h>
#include "vga_bios_term.h"
#include "multiboot.h"
#include "kmalloc.h"
#include "../../libc/include/util.h"

#define KERNEL_START        0xC0000000
#define KERNEL_HEAP_START   0xD0000000

#define REC_PAGE_DIR        (0xFFFFF000) // virt address of kernel_page_dir[1023], which leads to kernel_page_dir* itself
#define REC_PAGE_TABLE(i)   (0xFFC00000 + ((i)  << 12)) // virt address of the i-th entry of the kernel_page_table of kernel_page_dir[1023]

#define PAGE_SIZE 0x1000 // 4096 Bytes

#define PAGE_FLAG_PRESENT   (1 << 0)
#define PAGE_FLAG_WRITE     (1 << 1)
#define PAGE_FLAG_OWNER     (1 << 9)

#define TEMP_MAP_ADDR 0xE100000
/*
    0x10000000 = 4GB: entire 32-bit physical address space
    0x1000 = 4KB: page size
    4GB / 4KB = 1,048,576 possible page frames
*/
#define NUM_PAGE_FRAMES (0x100000000 / PAGE_SIZE)

extern uint32_t _kernel_end; // End of kernelcode in linker
extern uint32_t kernel_page_dir[1024]; // gets initialized in boot.s

typedef struct process_page_dir_header{
    unsigned int id;
    uint32_t page_dir_phys;    // what goes into "cr3"
    struct process_page_dir_header* next;
}process_page_dir_header_t;

void init_memory(mb_info_t* boot_info);
void invalidate_tlb_entry(uint32_t vaddr);
void pmm_init(uint32_t mem_low, uint32_t mem_high);
uint32_t pmm_alloc_page_frame();
void map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
uint32_t* get_page_dir_reg();
void change_page_dir_reg(uint32_t* page_dir);
uint32_t create_process_page_dir(unsigned int id);
void sync_page_dirs();
