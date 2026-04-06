#pragma once

#include <stdint.h>
#include <stddef.h>
#include "idt.h"
#include "bios_term.h"
#include "util.h"

void initPIT(int irq_id);
void pitHandler(intr_regs_t* regs);
void pitStart(uint32_t divisor);