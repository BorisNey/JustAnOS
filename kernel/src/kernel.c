#include "../include/vga_bios_term.h"
#include "../include/gdt.h"
#include "../include/idt.h"
#include "../include/keyboard.h"
#include "../include/pit.h"
#include "../include/multiboot.h"
#include "../include/memory.h"
#include "../include/kmalloc.h"
#include "../include/pci.h"

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
	createProcPageDir(3);

	while(1);

	return;
}
