#include "../include/util.h"

void outPortB(uint16_t port, uint8_t value){
	asm volatile("outb %1, %0" : : "dN" (port), "a" (value));
	return;
}

uint8_t inPortB(uint16_t port){
	uint8_t value;
	asm volatile("inb %1, %0" : "=a"(value) : "dN"(port));
	return value;
}

