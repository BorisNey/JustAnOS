#pragma once

#include <stddef.h>
#include <stdint.h>
#include "vga_terminal.h"
#include "../../libc/include/string.h"
#include "../../libc/include/util.h"

#define IDT_ENTRIES 256

typedef struct __attribute__((packed)) {
	uint16_t offset_low;
	uint16_t sel;
	uint8_t res;
	uint8_t flags;
	uint16_t offset_high;
} idt_entry_struct;

typedef struct __attribute__((packed)) {
	uint16_t limit;
	unsigned int base;
} idt_ptr_struct;

typedef struct __attribute__((packed)) {
	uint32_t cr2;
	uint32_t ds;
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t int_no, err_code;
	uint32_t eip, cs, eflags, user_esp, ss;
} intr_regs_struct;

void init_idt();
void set_idt_entry(unsigned int entry_index, uint32_t offset, 
		uint16_t sel, uint8_t gate_type, uint8_t dpl);
void isr_handler(intr_regs_struct* regs);
void install_irq_handler (int irq, 
		void (*handler)(intr_regs_struct* regs));
void uninstal_irq_handler (int irq);
void irq_handler(intr_regs_struct* regs);

// in idt_s.s
extern void idt_flush_asm(idt_ptr_struct* idt_addr);

// CPU Exceptions
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

// IRQs
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

// System calls
extern void isr128();
extern void isr177();
