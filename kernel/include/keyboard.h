#pragma once

#include <stdint.h>
#include <stddef.h>
#include "idt.h"
#include "vga_bios_term.h"
#include "../../libc/include/util.h"

#define KEY_PORT 0x60

void initKeyboard();
void keyboardHandlerDE(intr_regs_t* regs);
