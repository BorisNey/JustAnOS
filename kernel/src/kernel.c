#include "../include/vga_terminal.h"
#include "../include/gdt.h"
#include "../include/idt.h"
#include "../include/keyboard.h"

void kernel_init(){
	terminal_init(VGA_COLOR_BLACK, VGA_COLOR_CYAN);
	init_gdt();
	init_idt();
	install_irq_handler(1, keyboard_handler);
	return;
}

void kernel_main(){
	kernel_init();
	terminal_print("Hello World!\n");
	while(1);
	return;
}

