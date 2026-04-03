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

extern uint32_t g_kernel_end; // End of kernelcode in linker.ld
extern uint32_t g_kernel_page_dir[1024]; // gets initialized in boot.s

typedef struct proc_page_dir_header_t{
    unsigned int id;
    uint32_t page_dir_phys;    // what goes into "cr3"
    struct proc_page_dir_header_t* next;
}proc_page_dir_header_t;

void initMemory(mb_info_t* boot_info);
void invalidateTLBEntry(uint32_t vaddr);
void initPMM(uint32_t mem_low, uint32_t mem_high);
uint32_t allocPageFrame();
void mapAddr(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
uint32_t* getCurrPageDirReg();
void setCurrPageDirReg(uint32_t* page_dir);
uint32_t createProcPageDir(unsigned int id);
void syncPageDirs();
