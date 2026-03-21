#include "../include/keyboard.h"

void keyboard_handler(intr_regs_struct* regs){
	uint8_t scancode = inPortB(KEY_PORT);
	terminal_print("Key Pressed!\n");
	return;
}

