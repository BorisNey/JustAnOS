#pragma once

#include <stddef.h>
#include <stdint.h>

#define GDT_ENTRIES 5

typedef struct __attribute__((packed)) {
    uint16_t limit_low;     // limit_high are lower 4 bits of flags
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access_byte;
    uint8_t flags;
    uint8_t base_high;
} gdt_entry_struct;

typedef struct __attribute__((packed)) {
    uint16_t limit;
    unsigned int base;
} gdt_ptr_struct;

extern void gdt_flush_asm(gdt_ptr_struct* gdt_addr);
void init_gdt();
void set_gdt_entry(unsigned int entry_index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
