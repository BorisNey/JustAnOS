#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include "string.h"
#include "virt_addr.h"

/* Hardware text mode color constants. */
typedef enum {
	BT_COL_BLACK = 0,
	BT_COL_BLUE = 1,
	BT_COL_GREEN = 2,
	BT_COL_CYAN = 3,
	BT_COL_RED = 4,
	BT_COL_MAGENTA = 5,
	BT_COL_BROWN = 6,
	BT_COL_LIGHT_GREY = 7,
	BT_COL_DARK_GREY = 8,
	BT_COL_LIGHT_BLUE = 9,
	BT_COL_LIGHT_GREEN = 10,
	BT_COL_LIGHT_CYAN = 11,
	BT_COL_LIGHT_RED = 12,
	BT_COL_LIGHT_MAGENTA = 13,
	BT_COL_LIGHT_BROWN = 14,
	BT_COL_WHITE = 15,
} BIOS_TERM_COLOR;


void initBiosTerm(BIOS_TERM_COLOR background_color, BIOS_TERM_COLOR foreground_color);
void biosTermPutc(char c);	// !!! This cannot print € and § !!!
void biosTermPrintf(const char* string, ...);
