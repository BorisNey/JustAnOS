#include "../include/vga_terminal.h"

size_t bios_term_row;
size_t bios_term_column;
uint16_t bios_term_color;
uint16_t* bios_term_buffer;


void init_bios_term(VGA_COLOR background_color, 
		VGA_COLOR foreground_color){
	bios_term_buffer = (uint16_t*)VGA_MEMORY;
	bios_term_row = 0;
	bios_term_column = 0;
	bios_term_color = (background_color << 12) | (foreground_color << 8);

	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			bios_term_buffer[index] = bios_term_entry(' ');
		}
	}
	return;
}

uint16_t bios_term_entry(char c){
	return (uint16_t) bios_term_color | c;
}

void bios_term_scroll(){
	memcpy((uint16_t*)VGA_MEMORY, 
			(uint16_t*)VGA_MEMORY + VGA_WIDTH, 
			(size_t)VGA_WIDTH * VGA_HEIGHT * sizeof(uint16_t));
	bios_term_row--;
	return;
}

void bios_term_putc(char c){
	const size_t index = bios_term_row * VGA_WIDTH + bios_term_column;
	switch (c){
		case '\n':
			bios_term_row++;
			bios_term_column = 0;
			break;
		case '\t':
			bios_term_column += 3;
			break;
		default:
			bios_term_buffer[index] = bios_term_entry(c);
			bios_term_column++;
			break;
	}
	
	if (bios_term_column >= VGA_WIDTH) {
		bios_term_column = 0;
		bios_term_row++;
	}
	
	if (bios_term_row >= VGA_HEIGHT)
        bios_term_scroll();
  return;
}

void bios_term_print(const char* string){
	for (size_t i = 0; i < strlen(string); i++){
		bios_term_putc(string[i]);
	}
	return;
}

