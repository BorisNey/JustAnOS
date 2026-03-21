#pragma once

#include <stdint.h>
#include <stddef.h>
#include "idt.h"
#include "vga_terminal.h"
#include "../../libc/include/util.h"

#define KEY_PORT 0x60

void keyboard_handler(intr_regs_struct* regs);

