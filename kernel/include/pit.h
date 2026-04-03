#pragma once

#include <stdint.h>
#include <stddef.h>
#include "idt.h"
#include "vga_bios_term.h"
#include "../../libc/include/util.h"

#define CH0_DATA_PORT 0x40
#define CH1_DATA_PORT 0x41
#define CH2_DATA_PORT 0x42
#define CMD_PORT 0x43

void initPIT(uint32_t divisor);
void pitHandler(intr_regs_t* regs);

