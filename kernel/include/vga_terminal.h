#pragma once

#include <stddef.h>
#include <stdint.h>
#include "../../libc/include/string.h"
#include "../../libc/include/stdio.h"

/* Hardware text mode color constants. */
typedef enum {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
} VGA_COLOR;


#define VGA_WIDTH   80 
#define VGA_HEIGHT  25
#define VGA_MEMORY  0xB8000 

extern size_t terminal_row;
extern size_t terminal_column;
extern uint16_t terminal_color;
extern uint16_t* terminal_buffer;

static inline uint16_t terminal_entry(char c){
  return (uint16_t) terminal_color | c;
}

void terminal_init(VGA_COLOR background_color, VGA_COLOR foreground_color);
void terminal_scroll();
void terminal_putchar(char c);
void terminal_printf(const char* data);

