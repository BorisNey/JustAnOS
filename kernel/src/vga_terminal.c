#include "../include/vga_terminal.h"

size_t terminal_row;
size_t terminal_column;
uint16_t terminal_color;
uint16_t* terminal_buffer;


void terminal_init(VGA_COLOR background_color, VGA_COLOR foreground_color){
	terminal_buffer = (uint16_t*)VGA_MEMORY;
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = (background_color << 12) | (foreground_color << 8);

	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = terminal_entry(' ');
		}
	}
	return;
}

void terminal_scroll(){
	memcpy((uint16_t*)VGA_MEMORY, (uint16_t*)VGA_MEMORY+VGA_WIDTH, (size_t)VGA_WIDTH*VGA_HEIGHT);
	terminal_row--;
	return;
}

void terminal_putchar(char c){
	const size_t index = terminal_row * VGA_WIDTH + terminal_column;
	switch (c){
		case '\n':
			terminal_row++;
			terminal_column = 0;
			break;
		case '\t':
			terminal_column += 3;
			break;	
		default:
			terminal_buffer[index] = terminal_entry(c);
			terminal_column++;
			break;
	}
	
	if (terminal_column > VGA_WIDTH) {
		terminal_column = 0;
		terminal_row++;
	}
	
	if (terminal_row >= VGA_HEIGHT)
        terminal_scroll();
  return;
}

void terminal_print(const char* string){
	for (size_t i = 0; i < strlen(string); i++){
		terminal_putchar(string[i]);
	}
	return;
}

