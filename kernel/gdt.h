#pragma once

#include <stddef.h>
#include <stdint.h>

#define ENTRIES 5

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

void gdt_flush_asm(addr_t);
void init_gdt();
void set_gdt_entry(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

