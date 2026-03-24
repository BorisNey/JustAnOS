#pragma once

#include <stdint.h>
#include <stddef.h>
#include "memory.h"
#include "vga_bios_term.h"
#include "../../libc/include/util.h"

void init_kmalloc(uint32_t initial_heap_size);
void change_heap_size(uint32_t new_size);
