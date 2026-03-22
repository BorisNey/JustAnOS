#include "../include/vga_bios_term.h"
#include "../include/gdt.h"
#include "../include/idt.h"
#include "../include/keyboard.h"
#include "../include/pit.h"
#include "../include/multiboot.h"
#include "../include/memory.h"

void kernel_main(uint32_t magic, 
		multiboot_info_struct* boot_info){
	init_bios_term(VGA_COLOR_BLACK, VGA_COLOR_CYAN);
	init_gdt();
	init_idt();
	//init_pit(1073741823);
	init_keyboard();
	init_memory(boot_info);

	bios_term_print("Hello World!\n");
	while(1);

	return;
}
