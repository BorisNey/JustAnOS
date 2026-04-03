#pragma once

#include <stddef.h>
#include <stdint.h>
#include "vga_bios_term.h"
#include "../../libc/include/string.h"

#define GDT_ENTRIES 6

typedef struct __attribute__((packed)){
    uint16_t limit_low;     // limit_high are lower 4 bits of flags
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access_byte;
    uint8_t flags;
    uint8_t base_high;
}gdt_entry_t;

typedef struct __attribute__((packed)){
    uint16_t limit;
    unsigned int base;
}gdt_ptr_t;

typedef struct __attribute__((packed)){
	uint32_t prev_tss;
	uint32_t esp0;
	uint32_t ss0;
	uint32_t esp1;
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t fs;
	uint32_t gs;
	uint32_t ldt;
	uint32_t trap;
	uint32_t iomap_base;
}tss_entry_t;

extern void gdtFlush(gdt_ptr_t* gdt_addr);
extern void tssFlush();

void initGDT();
void setGdtEentry(unsigned int entry_index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
void setTssEntry(unsigned int entry_index, uint16_t ss0, uint32_t esp0);

