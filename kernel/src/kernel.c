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

	pci_dev_t* pci_dev = pciFindDevType(0x02, 0x00, 0xFF);
	if(pci_dev == NULL){
		biosTermPrintf("No Ethernet Controller found\n");
	}
	else{
		biosTermPrintf("Device ID: %x, Vendor ID: %x", 
			(uint32_t)(pci_dev->dev_spec.device_id), (uint32_t)(pci_dev->dev_spec.vendor_id));
	}

	while(1);

	return;
}
