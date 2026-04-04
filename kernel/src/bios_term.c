#include "bios_term.h"

static size_t bios_term_row;
static size_t bios_term_column;
static uint16_t bios_term_color;
static uint16_t* bios_term_buffer;

static uint16_t biosTermEntry(char c){
	return (uint16_t) bios_term_color | c;
}

static void biosTermScroll(){
	memcpy((uint16_t*)VGA_MEMORY, 
			(uint16_t*)VGA_MEMORY + VGA_WIDTH, 
			(size_t)VGA_WIDTH * VGA_HEIGHT * sizeof(uint16_t));
	bios_term_row--;
	return;
}

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

/*
For now only support for %d and %x
*/
__attribute__((format(printf, 1, 2))) void biosTermPrintf(const char* string, ...){
	va_list args;
	va_start(args, string);

	int i = 0;
	while (string[i] != 0){
		if (string[i] == '%' && (string[i + 1] == 'x' || string[i + 1] == 'd')){
			int j = 0;
			char format_string[12];

			switch(string[i + 1]){
				case 'd':
					itoa(va_arg(args, int), format_string);
					break;
				case 'x':
					htoa(va_arg(args, uint32_t), format_string);
					break;
			}

			while (format_string[j] != 0){
				biosTermPutc(format_string[j]);
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

