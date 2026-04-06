#pragma once

#define KERNEL_START                0xC0000000

#define VGA_MEMORY                  0xC00B8000

#define KERNEL_HEAP_START           0xD0000000 // 536,870912 MB until 0xF0000000

#define TEMP_PROC_PD_ADDR           0xF0000000  // A temporary Page Directory Address to set up the recursive entry
// Takes 1P             -> + 0x1000 0xF0001000
#define e1000_MMIO_REG_SPACE_ADDR   0xF1000000  // Start of the e1000 MMIO Register Space
// Takes 32P            -> +0x20000 0xF1020000
#define TX_DESCRIPTORS              0xF2000000
// Takes 1P             -> +0x1000  0xF2001000
#define TX_BUFFERS                  0xF2001000
// Takes 8P             -> + 0x8000 0xF2009000
#define RX_DESCRIPTORS              0xF2009000
// Takes 1P             -> +0x1000  0xF200A000
#define RX_BUFFERS                  0xF200A000
// Takes 32P            -> +0x20000 0xF202A000
