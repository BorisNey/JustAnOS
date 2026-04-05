#pragma once

#include <stdint.h>
#include <stddef.h>
#include "idt.h"
#include "bios_term.h"
#include "util.h"

void initKeyboard(int irq_id);
