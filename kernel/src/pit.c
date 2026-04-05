#include "pit.h"

#define CH0_DATA_PORT 0x40
#define CH1_DATA_PORT 0x41
#define CH2_DATA_PORT 0x42
#define CMD_PORT 0x43

static void pitHandler(intr_regs_t* regs){
	UNUSED(regs);
	
	biosTermPrintf("Tick!\n");
	return;
}

void initPIT(uint32_t divisor){
	installIrqHandler(0, pitHandler);
	outPortB(CMD_PORT, 0x36);
	outPortB(0x40, (uint8_t)(divisor & 0xFF));
	outPortB(0x40, (uint8_t)((divisor >> 8) & 0xFF));
	biosTermPrintf("DBG: PIT init success\n");
	return;
}
