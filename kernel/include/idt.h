#pragma once

#include <stddef.h>
#include <stdint.h>
#include "bios_term.h"
#include "string.h"
#include "util.h"

#define IDT_ENTRIES 256

typedef struct __attribute__((packed)){
	uint16_t offset_low;
	uint16_t sel;
	uint8_t res;
	uint8_t flags;
	uint16_t offset_high;
} idt_entry_t;

typedef struct __attribute__((packed)){
	uint16_t limit;
	unsigned int base;
} idt_ptr_t;

typedef struct __attribute__((packed)){
	uint32_t cr2;
	uint32_t ds;
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t int_no, err_code;
	uint32_t eip, cs, eflags, user_esp, ss;
} intr_regs_t;

void initIDT();
void isrHandler(intr_regs_t* regs);
void installIrqHandler (int irq, void (*handler)(intr_regs_t* regs));
void uninstallIrqHandler (int irq);
void irqHandler(intr_regs_t* regs);
