#include "../include/keyboard.h"

void init_keyboard(){
	install_irq_handler(1, keyboard_handler);
	return;
}

void keyboard_handler(intr_regs_struct* regs){
	uint8_t scancode = inPortB(KEY_PORT);
	bios_term_print("Key Pressed!\n");
	return;
}

