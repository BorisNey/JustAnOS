#include "bios_term.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "pit.h"
#include "multiboot.h"
#include "memory.h"
#include "kmalloc.h"
#include "pci.h"

void kernel_main(mb_info_t* boot_info){
	initBiosTerm(VGA_COLOR_BLACK, VGA_COLOR_CYAN);
	initGDT();
	initIDT();
	//initPIT(1073741823);
	initKeyboard();
	initMemory(boot_info);
	initKmalloc(PAGE_SIZE);
	initPCI();

	biosTermPrintf("\nHello World!\n");
	enumeratePCI();

	while(1);

	return;
}
