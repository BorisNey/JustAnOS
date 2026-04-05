#pragma once

#include <stdint.h>
#include <stddef.h>
#include "bios_term.h"
#include "kmalloc.h"
#include "pci.h"
#include "util.h"


void netRecievePacket(void* data, uint32_t size);
