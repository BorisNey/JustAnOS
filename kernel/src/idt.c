#include "idt.h"

// in idt_s.s

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

extern void idtFlush(idt_ptr_t* idt_addr);


const char* exception_messages[] = {
	"Division By Zero",				// 0
	"Debug",						// 1
	"Non Maskable Interrupt",		// 2
	"Breakpoint",					// 3
	"Into Detected Overflow",		// 4
	"Out Of Bounds",				// 5
	"Invalid Opcode",				// 6
	"No Coprocessor",				// 7
	"Double Fault",					// 8
	"Coprocessor Segement Overrun",	// 9
	"Bad TSS",						// 10
	"Segment Not Present",			// 11
	"Stack Fault",					// 12
	"General Protection Fault",		// 13
	"Page Fault",					// 14
	"Unknown Interrupt",			// 15
	"Coprocessor Fault",			// 16
	"Aligment Fault",				// 17
	"Machiene Check",				// 18
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

void* irq_handlers[16] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

idt_entry_t idt_entries[IDT_ENTRIES];
idt_ptr_t idt_ptr;

static void setIdtEntry(unsigned int entry_index, uint32_t offset, 
		uint16_t sel, uint8_t gate_type, uint8_t dpl){
	idt_entries[entry_index].offset_low = offset & 0xFFFF;
	idt_entries[entry_index].sel = sel;
	idt_entries[entry_index].res = 0;
	idt_entries[entry_index].flags = 0x80 | ((dpl << 5) & 0x60) | (gate_type & 0x0F);
	idt_entries[entry_index].offset_high = (offset >> 16) & 0xFFFF;
	
	return;
}

void initIDT(){
	idt_ptr.limit = sizeof(idt_entry_t) * IDT_ENTRIES - 1;
	idt_ptr.base = (unsigned int)&idt_entries;
	
	memset(&idt_entries, 0, sizeof(idt_entry_t) * IDT_ENTRIES);
	
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
	setIdtEntry(0, (uint32_t)isr0, 0x08, 0xE, 0);
	setIdtEntry(1, (uint32_t)isr1, 0x08, 0xE, 0);
	setIdtEntry(2, (uint32_t)isr2, 0x08, 0xE, 0);
	setIdtEntry(3, (uint32_t)isr3, 0x08, 0xE, 0);
	setIdtEntry(4, (uint32_t)isr4, 0x08, 0xE, 0);
	setIdtEntry(5, (uint32_t)isr5, 0x08, 0xE, 0);
	setIdtEntry(6, (uint32_t)isr6, 0x08, 0xE, 0);
	setIdtEntry(7, (uint32_t)isr7, 0x08, 0xE, 0);
	setIdtEntry(8, (uint32_t)isr8, 0x08, 0xE, 0);
	setIdtEntry(9, (uint32_t)isr9, 0x08, 0xE, 0);
	setIdtEntry(10, (uint32_t)isr10, 0x08, 0xE, 0);
	setIdtEntry(11, (uint32_t)isr11, 0x08, 0xE, 0);
	setIdtEntry(12, (uint32_t)isr12, 0x08, 0xE, 0);
	setIdtEntry(13, (uint32_t)isr13, 0x08, 0xE, 0);
	setIdtEntry(14, (uint32_t)isr14, 0x08, 0xE, 0);
	setIdtEntry(15, (uint32_t)isr15, 0x08, 0xE, 0);
	setIdtEntry(16, (uint32_t)isr16, 0x08, 0xE, 0);
	setIdtEntry(17, (uint32_t)isr17, 0x08, 0xE, 0);
	setIdtEntry(18, (uint32_t)isr18, 0x08, 0xE, 0);
	setIdtEntry(19, (uint32_t)isr19, 0x08, 0xE, 0);
	setIdtEntry(20, (uint32_t)isr20, 0x08, 0xE, 0);
	setIdtEntry(21, (uint32_t)isr21, 0x08, 0xE, 0);
	setIdtEntry(22, (uint32_t)isr22, 0x08, 0xE, 0);
	setIdtEntry(23, (uint32_t)isr23, 0x08, 0xE, 0);
	setIdtEntry(24, (uint32_t)isr24, 0x08, 0xE, 0);
	setIdtEntry(25, (uint32_t)isr25, 0x08, 0xE, 0);
	setIdtEntry(26, (uint32_t)isr26, 0x08, 0xE, 0);
	setIdtEntry(27, (uint32_t)isr27, 0x08, 0xE, 0);
	setIdtEntry(28, (uint32_t)isr28, 0x08, 0xE, 0);
	setIdtEntry(29, (uint32_t)isr29, 0x08, 0xE, 0);
	setIdtEntry(30, (uint32_t)isr30, 0x08, 0xE, 0);
	setIdtEntry(31, (uint32_t)isr31, 0x08, 0xE, 0);
	
	// IRQs
	setIdtEntry(32, (uint32_t)irq0, 0x08, 0xE, 0);
	setIdtEntry(33, (uint32_t)irq1, 0x08, 0xE, 0);
	setIdtEntry(34, (uint32_t)irq2, 0x08, 0xE, 0);
	setIdtEntry(35, (uint32_t)irq3, 0x08, 0xE, 0);
	setIdtEntry(36, (uint32_t)irq4, 0x08, 0xE, 0);
	setIdtEntry(37, (uint32_t)irq5, 0x08, 0xE, 0);
	setIdtEntry(38, (uint32_t)irq6, 0x08, 0xE, 0);
	setIdtEntry(39, (uint32_t)irq7, 0x08, 0xE, 0);
	setIdtEntry(40, (uint32_t)irq8, 0x08, 0xE, 0);
	setIdtEntry(41, (uint32_t)irq9, 0x08, 0xE, 0);
	setIdtEntry(42, (uint32_t)irq10, 0x08, 0xE, 0);
	setIdtEntry(43, (uint32_t)irq11, 0x08, 0xE, 0);
	setIdtEntry(44, (uint32_t)irq12, 0x08, 0xE, 0);
	setIdtEntry(45, (uint32_t)irq13, 0x08, 0xE, 0);
	setIdtEntry(46, (uint32_t)irq14, 0x08, 0xE, 0);
	setIdtEntry(47, (uint32_t)irq15, 0x08, 0xE, 0);

	// System calls
	setIdtEntry(128, (uint32_t)isr128, 0x08, 0xE, 3);
	setIdtEntry(177, (uint32_t)isr177, 0x08, 0xE, 3);
	
	idtFlush(&idt_ptr);
	
	biosTermPrintf("DBG: IDT init success\n");
	return;
}

void isrHandler(intr_regs_t* regs){
	if (regs->int_no < 32){
		biosTermPrintf("KERNEL PANIC! CPU Exception: ");
		biosTermPrintf(exception_messages[regs->int_no]);
		biosTermPrintf("\n");
		while(1);
	}
	return;
}

void installIrqHandler (int irq, 
		void (*handler)(intr_regs_t* regs)){
	irq_handlers[irq] = handler;
	return;
}

void uninstallIrqHandler (int irq){
	irq_handlers[irq] = 0;
	return;
}

void irqHandler(intr_regs_t* regs){
	// Pointer to function "handler" that returns and int and takes "regs" as par
	void (*handler)(intr_regs_t* regs);
	
	if (regs->int_no < 32 || regs->int_no > 47)
		return;

	handler = irq_handlers[regs->int_no - 32];
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

