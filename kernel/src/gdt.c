#include "../include/gdt.h"

gdt_entry_t gdt_entries[GDT_ENTRIES];
gdt_ptr_t gdt_ptr;
tss_entry_t tss_entry;

void initGDT(){
    gdt_ptr.limit = (sizeof(gdt_entry_t) * GDT_ENTRIES) - 1;
    gdt_ptr.base = (unsigned int)&gdt_entries;

    setGdtEentry(0, 0, 0, 0, 0); // NULL
    setGdtEentry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // kernel code
    setGdtEentry(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // kernel data
    setGdtEentry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // user code
    setGdtEentry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // user data
    /*
    BUG:
    When a ring 3 -> ring 0 privilege switch occurs (any interrupt from user mode), 
    the CPU loads esp0 from the TSS. Address 0 is unmapped, so this will immediately triple-fault.
    Not a problem now since no user mode.
    */
    setTssEntry(5, 0x10, 0x0);
    
    gdtFlush(&gdt_ptr);
    tssFlush();

    biosTermPrintf("DBG: GDT/TSS init success\n");

    return;
}

void setGdtEentry(unsigned int entry_index, uint32_t base,
		uint32_t limit, uint8_t access_byte, uint8_t gran){
  if (limit > 0xFFFFFFFF) {
        biosTermPrintf("ERROR: Source limit is larger than 0xFFFFF\n");
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

void setTssEntry(unsigned int entry_index, uint16_t ss0, uint32_t esp0){
	uint32_t base = (uint32_t) &tss_entry;
	uint32_t limit = base + sizeof(tss_entry_t) - 1;
	
	setGdtEentry(entry_index, base, limit, 0xE9, 0x00);

	memset(&tss_entry, 0, sizeof(tss_entry_t));
	tss_entry.ss0 = ss0;
	tss_entry.esp0 = esp0;
	tss_entry.cs = 0x08 | 0x3;
	tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x10 | 0x3;

	return;
}

