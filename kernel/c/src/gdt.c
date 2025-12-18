#include "../include/gtd.h"

void gdt_flush_asm(addr_t) {
    __asm__("global gdt_flush       \n"
            "gdt_flush:             \n"
            "   MOV eax, [esp+4]    \n"
            "   LGDT [eax]          \n"
            "   MOV eax, 0x10       \n"
            "   MOV ds, ax          \n"
            "   MOV es, ax          \n"
            "   MOV fs, ax          \n"
            "   MOV gs, ax          \n"
            "   MOV ss, ax          \n"
            "   JMP 0x08:.flush     \n"
            ".flush:                \n"
            "   RET                 \n"
    );
};

gdt_entry_struct[ENTRIES] gdt_entries;
gdt_ptr_struct gdt_ptr;

void init_gdt(){
    gdt_ptr.limit = (sizeof(gdt_entry_struct) * ENTRIES) - 1;
    gdt_ptr.base = &gdt_entries;

    set_gdt_entry(0, 0, 0, 0, 0); // NULL
    set_gdt_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // kernel code
    set_gdt_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // kernel data
    set_gdt_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // user code
    set_gdt_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // user data
    
    gdt_flush_asm(&gdt_ptr);
};

void set_gdt_entry(uint32_t entry_num, uint32_t base, uint32_t limit, uint8_t access_byte, uint8_t gran){
    if (limit > 0xFFFFFFFF) {
        terminal_printf("source limit is larger than 0xFFFFF\n");
        return;
    }

    gdt_entries[num].limit_low = (limit & 0xFFFF);
    gdt_entries[num].base_low = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].access_byte = access_byte;
    gdt_entries[num].flags = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt_entries[num].base_high = (base >> 24) & 0xFF;
}

