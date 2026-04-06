#pragma once

#include <stdint.h>
#include <stddef.h>
#include "bios_term.h"
#include "kmalloc.h"
#include "pci.h"

#include "keyboard.h"
#include "i8254x.h"

#define INTEL_VENDOR_ID 0x8086
#define AMD_VENDOR_ID 0x1022

void initDrivers();
