#include "../include/vga_bios_term.h"

size_t bios_term_row;
size_t bios_term_column;
uint16_t bios_term_color;
uint16_t* bios_term_buffer;


void initBiosTerm(VGA_COLOR background_color, VGA_COLOR foreground_color){
	bios_term_buffer = (uint16_t*)VGA_MEMORY;
	bios_term_row = 0;
	bios_term_column = 0;
	bios_term_color = (background_color << 12) | (foreground_color << 8);

	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			bios_term_buffer[index] = biosTermEntry(' ');
		}
	}
	biosTermPrintf("DBG: BIOS Terminal init success\n");
	return;
}

uint16_t biosTermEntry(char c){
	return (uint16_t) bios_term_color | c;
}

void biosTermScroll(){
	memcpy((uint16_t*)VGA_MEMORY, 
			(uint16_t*)VGA_MEMORY + VGA_WIDTH, 
			(size_t)VGA_WIDTH * VGA_HEIGHT * sizeof(uint16_t));
	bios_term_row--;
	return;
}

void biosTermPutc(char c){
	size_t index = bios_term_row * VGA_WIDTH + bios_term_column;
	switch (c){
		case '\n':
			bios_term_row++;
			bios_term_column = 0;
			break;
		case '\t':
			bios_term_column += 3;
			break;
		case '\b':
			bios_term_buffer[--index] = biosTermEntry(' ');
			if(bios_term_column == 0){
				bios_term_column = VGA_WIDTH - 1;
				bios_term_row--;
			}
			else{
				bios_term_column--;
			}
			break;
		default:
			bios_term_buffer[index] = biosTermEntry(c);
			bios_term_column++;
			break;
	}
	
	if (bios_term_column >= VGA_WIDTH) {
		bios_term_column = 0;
		bios_term_row++;
	}
	
	if (bios_term_row >= VGA_HEIGHT)
        biosTermScroll();
  return;
}

// __attribute__((format(printf, 1, 2)))
void biosTermPrintf(const char* string, ...){
	va_list args;
	va_start(args, string);

	int i = 0;
	while (string[i] != 0){
		if (string[i] == '%' && string[i + 1] == 'd'){
			char number_string[12];
			itoa(va_arg(args, int), number_string);
			int j = 0;
			while (number_string[j] != 0){
				biosTermPutc(number_string[j]);
				j++;
			}
			i += 2;
		}
		else{
			biosTermPutc(string[i]);
			i++;
		}
	}

	va_end(args);
	return;
}

