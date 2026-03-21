#include "../include/idt.h"

idt_entry_struct idt_entries[IDT_ENTRIES];
idt_ptr_struct idt_ptr;

char* exception_messages[] = {
	"Division By Zero",
	"Debug",
	"Non Maskable Interrupt",
	"Breakpoint",
	"Into Detected Overflow",
	"Out Of Bounds",
	"Invalid Opcode",
	"No Coprocessor",
	"Double Fault",
	"Coprocessor Segement Overrun",
	"Bad TSS",
	"Segment Not Present",
	"Stack Fault",
	"General Protection Fault",
	"Page Fault",
	"Unknown Interrupt",
	"Coprocessor Fault",
	"Aligment Fault",
	"Machiene Check",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

void* irq_handler_list[16] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

void init_idt(){
	idt_ptr.limit = sizeof(idt_entry_struct) * IDT_ENTRIES - 1;
	idt_ptr.base = (unsigned int)&idt_entries;
	
	memset(&idt_entries, 0, sizeof(idt_entry_struct) * IDT_ENTRIES);
	
	/*
	Two PIC Chips:
		1. Master: 0x20 for commands and 0x21 for data
		2. Slave: 0xA0 for commands and 0xA1 for data
	*/
	
	/*
	ICW1: Put chips initialization mode:
		Bit 0: more config word are comming
		Bit 4: initialization mode
	*/
	outPortB(0x20, 0x11);
	outPortB(0xA0, 0x11);
	
	/*
	ICW2: Remap Vector Offsets for IRQs:
		0x00-0x1F: CPU exceptions
		0x20: timer
		0x21: keyboard
		...
		0x28: RTC
		...
	*/
	outPortB(0x21, 0x20);
	outPortB(0xA1, 0x28);
	
	/*
	ICW3: Setting Relations between PIC:
		Bit 3: Master is connected to IRQ line 2
		Bit 1: Slaves output is connected to IRQ line 2
	*/
	outPortB(0x21, 0x04);
	outPortB(0xA1, 0x02);
	
	/*
	ICW4: Setting the Operating Mode:
		Bit 1: 8086/8088 Mode (required for modern x86 systems)
	*/
	outPortB(0x21, 0x01);
	outPortB(0xA1, 0x01);
	
	/*
	OCW1: Unmask all interrupts:
		Enable all IRQs
	*/
	outPortB(0x21, 0x0);
	outPortB(0xA1, 0x0);
	
	//CPU Exception
	set_idt_entry(0, (uint32_t)isr0, 0x08, 0xE, 0);
	set_idt_entry(1, (uint32_t)isr1, 0x08, 0xE, 0);
	set_idt_entry(2, (uint32_t)isr2, 0x08, 0xE, 0);
	set_idt_entry(3, (uint32_t)isr3, 0x08, 0xE, 0);
	set_idt_entry(4, (uint32_t)isr4, 0x08, 0xE, 0);
	set_idt_entry(5, (uint32_t)isr5, 0x08, 0xE, 0);
	set_idt_entry(6, (uint32_t)isr6, 0x08, 0xE, 0);
	set_idt_entry(7, (uint32_t)isr7, 0x08, 0xE, 0);
	set_idt_entry(8, (uint32_t)isr8, 0x08, 0xE, 0);
	set_idt_entry(9, (uint32_t)isr9, 0x08, 0xE, 0);
	set_idt_entry(10, (uint32_t)isr10, 0x08, 0xE, 0);
	set_idt_entry(11, (uint32_t)isr11, 0x08, 0xE, 0);
	set_idt_entry(12, (uint32_t)isr12, 0x08, 0xE, 0);
	set_idt_entry(13, (uint32_t)isr13, 0x08, 0xE, 0);
	set_idt_entry(14, (uint32_t)isr14, 0x08, 0xE, 0);
	set_idt_entry(15, (uint32_t)isr15, 0x08, 0xE, 0);
	set_idt_entry(16, (uint32_t)isr16, 0x08, 0xE, 0);
	set_idt_entry(17, (uint32_t)isr17, 0x08, 0xE, 0);
	set_idt_entry(18, (uint32_t)isr18, 0x08, 0xE, 0);
	set_idt_entry(19, (uint32_t)isr19, 0x08, 0xE, 0);
	set_idt_entry(20, (uint32_t)isr20, 0x08, 0xE, 0);
	set_idt_entry(21, (uint32_t)isr21, 0x08, 0xE, 0);
	set_idt_entry(22, (uint32_t)isr22, 0x08, 0xE, 0);
	set_idt_entry(23, (uint32_t)isr23, 0x08, 0xE, 0);
	set_idt_entry(24, (uint32_t)isr24, 0x08, 0xE, 0);
	set_idt_entry(25, (uint32_t)isr25, 0x08, 0xE, 0);
	set_idt_entry(26, (uint32_t)isr26, 0x08, 0xE, 0);
	set_idt_entry(27, (uint32_t)isr27, 0x08, 0xE, 0);
	set_idt_entry(28, (uint32_t)isr28, 0x08, 0xE, 0);
	set_idt_entry(29, (uint32_t)isr29, 0x08, 0xE, 0);
	set_idt_entry(30, (uint32_t)isr30, 0x08, 0xE, 0);
	set_idt_entry(31, (uint32_t)isr31, 0x08, 0xE, 0);
	
	// IRQs
	set_idt_entry(32, (uint32_t)irq0, 0x08, 0xE, 0);
	set_idt_entry(33, (uint32_t)irq1, 0x08, 0xE, 0);
	set_idt_entry(34, (uint32_t)irq2, 0x08, 0xE, 0);
	set_idt_entry(35, (uint32_t)irq3, 0x08, 0xE, 0);
	set_idt_entry(36, (uint32_t)irq4, 0x08, 0xE, 0);
	set_idt_entry(37, (uint32_t)irq5, 0x08, 0xE, 0);
	set_idt_entry(38, (uint32_t)irq6, 0x08, 0xE, 0);
	set_idt_entry(39, (uint32_t)irq7, 0x08, 0xE, 0);
	set_idt_entry(40, (uint32_t)irq8, 0x08, 0xE, 0);
	set_idt_entry(41, (uint32_t)irq9, 0x08, 0xE, 0);
	set_idt_entry(42, (uint32_t)irq10, 0x08, 0xE, 0);
	set_idt_entry(43, (uint32_t)irq11, 0x08, 0xE, 0);
	set_idt_entry(44, (uint32_t)irq12, 0x08, 0xE, 0);
	set_idt_entry(45, (uint32_t)irq13, 0x08, 0xE, 0);
	set_idt_entry(46, (uint32_t)irq14, 0x08, 0xE, 0);
	set_idt_entry(47, (uint32_t)irq15, 0x08, 0xE, 0);

	// System calls
	set_idt_entry(128, (uint32_t)isr128, 0x08, 0xE, 0);
	set_idt_entry(177, (uint32_t)isr177, 0x08, 0xE, 0);
	
	idt_flush_asm(&idt_ptr);
	
	terminal_print("DBG: IDT load success\n");
	return;
}

void set_idt_entry(unsigned int entry_index, uint32_t offset, 
		uint16_t sel, uint8_t gate_type, uint8_t dpl){
	idt_entries[entry_index].offset_low = offset & 0xFFFF;
	idt_entries[entry_index].sel = sel;
	idt_entries[entry_index].res = 0;
	idt_entries[entry_index].flags = 0x80 | ((dpl << 5) & 0x60) | (gate_type & 0x0F);
	idt_entries[entry_index].offset_high = (offset >> 16) & 0xFFFF;
	
	return;
}

void isr_handler(intr_regs_struct* regs){
	if (regs->int_no < 32){
		terminal_print("System halt! Exception: ");
		terminal_print(exception_messages[regs->int_no]);
		terminal_print("\n");
		while(1);
	}
	return;
}

void install_irq_handler (int irq, 
		void (*handler)(intr_regs_struct* regs)){
	irq_handler_list[irq] = handler;
	return;
}

void uninstal_irq_handler (int irq){
	irq_handler_list[irq] = 0;
	return;
}

void irq_handler(intr_regs_struct* regs){
	// Pointer to function "handler" that returns and int and takes "regs" as par
	void (*handler)(intr_regs_struct* regs);
	
	if (regs->int_no < 32 || regs->int_no > 47)
		return;

	handler = irq_handler_list[regs->int_no - 32];
	if (handler != 0){
		handler(regs);
	}
	
	/*
	0x20: EOI (End Of Interrupt):
		If INTR came from Slave, EIO must be send to both Master and Slave
	*/
	if (regs->int_no >= 40){
		outPortB(0xA0, 0x20);
	}
	outPortB(0x20, 0x20);
	return;
}

