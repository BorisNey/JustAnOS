#pragma once

#include <stdint.h>
#include <stddef.h>
#include "idt.h"
#include "bios_term.h"
#include "util.h"

#define KEY_PORT 0x60

void initKeyboard();
