#include "../include/vga_terminal.h"
#include "../include/gdt.h"
#include "../include/idt.h"
#include "../include/keyboard.h"
#include "../include/pit.h"

void kernel_init(){
	init_bios_term(VGA_COLOR_BLACK, VGA_COLOR_CYAN);
	init_gdt();
	init_idt();
	init_pit(1073741823);
	init_keyboard();
	return;
}

void kernel_main(){
	kernel_init();
	bios_term_print("Hello World!\n");
	while(1);
	return;
}

