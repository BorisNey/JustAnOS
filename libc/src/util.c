#include "util.h"

void outPortB(uint16_t port, uint8_t value){
	asm volatile("outb %1, %0" : : "dN" (port), "a" (value));
	return;
}

uint8_t inPortB(uint16_t port){
	uint8_t value;
	asm volatile("inb %1, %0" : "=a"(value) : "dN"(port));
	return value;
}

void outPortL(uint16_t port, uint32_t value){
	asm volatile("outl %1, %0" : : "dN" (port), "a" (value));
	return;
}

uint32_t inPortL(uint16_t port){
	uint32_t value;
	asm volatile("inl %1, %0" : "=a"(value) : "dN"(port));
	return value;
}
