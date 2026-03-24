#include "../include/pit.h"

void init_pit(uint32_t divisor){
	install_irq_handler(0, pit_handler);
	outPortB(CMD_PORT, 0x36);
	outPortB(0x40, (uint8_t)(divisor & 0xFF));
	outPortB(0x40, (uint8_t)((divisor >> 8) & 0xFF));
	bios_term_print("DBG: PIT initialization success\n");
	return;
}

void pit_handler(intr_regs_struct* regs){
	UNUSED(regs);
	
	bios_term_print("Tick!\n");
	return;
}
