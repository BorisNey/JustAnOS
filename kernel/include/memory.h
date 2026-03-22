#pragma once

#include <stddef.h>
#include <stdint.h>
#include "vga_bios_term.h"
#include "multiboot.h"

void init_memory(multiboot_info_struct* boot_info);

