#include "../include/gdt.h"
#include "../include/vga_terminal.h"

gdt_entry_struct gdt_entries[GDT_ENTRIES];
gdt_ptr_struct gdt_ptr;

void init_gdt(){
    gdt_ptr.limit = (sizeof(gdt_entry_struct) * GDT_ENTRIES) - 1;
    gdt_ptr.base = (unsigned int)&gdt_entries;

    set_gdt_entry(0, 0, 0, 0, 0); // NULL
    set_gdt_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // kernel code
    set_gdt_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // kernel data
    set_gdt_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // user code
    set_gdt_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // user data
    
    gdt_flush_asm(&gdt_ptr);
    terminal_print("DBG: GDT load success\n");

    return;
}

void set_gdt_entry(unsigned int entry_index, uint32_t base, uint32_t limit, uint8_t access_byte, uint8_t gran){
    if (limit > 0xFFFFFFFF) {
        terminal_print("Error: Source limit is larger than 0xFFFFF\n");
        return;
    }

    gdt_entries[entry_index].limit_low = (limit & 0xFFFF);
    gdt_entries[entry_index].base_low = (base & 0xFFFF);
    gdt_entries[entry_index].base_middle = (base >> 16) & 0xFF;
    gdt_entries[entry_index].access_byte = access_byte;
    gdt_entries[entry_index].flags = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt_entries[entry_index].base_high = (base >> 24) & 0xFF;

    return;
}
