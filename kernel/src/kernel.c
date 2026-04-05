#include "bios_term.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "pit.h"
#include "multiboot.h"
#include "memory.h"
#include "kmalloc.h"
#include "pci.h"
#include "drivers.h"

void kernel_main(mb_info_t* boot_info){
	initBiosTerm(BT_COL_BLACK, BT_COL_CYAN);
	initGDT();
	initIDT();
	initMemory(boot_info);
	initKmalloc(PAGE_SIZE);
	initPCI();
	initDrivers();

	biosTermPrintf("\nHello World!\n");

	while(1) __asm__ ("hlt");

	return;
}
