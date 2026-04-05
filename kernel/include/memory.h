#pragma once

#include <stddef.h>
#include <stdint.h>
#include "virt_addr.h"
#include "bios_term.h"
#include "multiboot.h"
#include "kmalloc.h"
#include "kll.h"
#include "util.h"

#define PAGE_SIZE 0x1000 // 4096 Bytes

#define PAGE_FLAG_PRESENT   (1 << 0)
#define PAGE_FLAG_WRITE     (1 << 1)
#define PAGE_FLAG_OWNER     (1 << 9)


typedef struct proc_pd_header_t{
    unsigned int id;
    uint32_t page_dir_phys;    // what goes into "cr3"
}proc_pd_header_t;

void initMemory(mb_info_t* boot_info);
void invalidateTLBEntry(uint32_t vaddr);
void initPMM(uint32_t mem_low, uint32_t mem_high);
uint32_t allocPageFrame();
void mapAddr(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
uint32_t* getCurrPageDirReg();
void setCurrPageDirReg(uint32_t* page_dir);
proc_pd_header_t* createProcPageDir(unsigned int id);
void syncPageDirs();
