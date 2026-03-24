#pragma once

#include <stddef.h>
#include <stdint.h>

#define FALSE 0
#define TRUE 1

#define CEIL_DIV(a, b) (((a + b) - 1) / b)

void outPortB(uint16_t port, uint8_t value);
uint8_t inPortB(uint16_t port);

