#include "pit.h"

#define CMD_PORT 0x43
#define CH0_DATA_PORT 0x40
#define CH1_DATA_PORT 0x41
#define CH2_DATA_PORT 0x42

// Bits 7 and 6
#define SEL_CH0 (0x0 << 6)
#define SEL_CH1	(0x1 << 6)
#define SEL_CH2 (0x2 << 6)

// Bits 5 and 4
#define LCV (0x0 << 4)	// Latch count value
#define LB (0x1 << 4)	// Low Byte Only
#define HB (0x2 << 4)	// High Byte Only
#define LHB	(0x3 << 4)	// Low/High Byte

// Bits 3 to 1
#define MD0	(0x0 << 1)	// interrupt on terminal count
#define MD1	(0x1 << 1)	// hardware re-triggerable one-shot
#define MD2	(0x2 << 1)	// rate generator
#define MD3	(0x3 << 1)	// square wave generator
#define MD4	(0x4 << 1)	// software triggered strobe
#define MD5	(0x5 << 1)	// hardware triggered strobe

// Bit 0
#define BIN (0x0 << 0)	// 16 Bit binary
#define BCD (0x1 << 0)	// BCD


void initPIT(){
	installIrqHandler(0, pitHandler);
	biosTermPrintf("DBG: PIT init success\n");
	return;
}

void pitHandler(intr_regs_t* regs){
	UNUSED(regs);
	biosTermPrintf("Tick!\n");
	return;
}

void pitStart(uint32_t divisor){
	uint8_t cmd = BIN | MD3 | LHB | SEL_CH0;
	outPortB(CMD_PORT, cmd);
	outPortB(CH0_DATA_PORT, (uint8_t)(divisor & 0xFF));
	outPortB(CH0_DATA_PORT, (uint8_t)((divisor >> 8) & 0xFF));
	return;
}

