#pragma once

#include <stdint.h>
#include <stddef.h>
#include "virt_addr.h"
#include "bios_term.h"
#include "idt.h"
#include "kmalloc.h"
#include "pci.h"
#include "util.h"
#include "net.h"

#define i82540EM_DEVICE_ID 0x100E
#define i82540EP_DEVICE_ID 0x1017
#define i82545EM_DEVICE_ID 0x100F
#define i82545GM_DEVICE_ID 0x1026
#define i82546EB_DEVICE_ID 0x1010
#define i82541EI_DEVICE_ID 0x1013
#define i82541GI_DEVICE_ID 0x1076
#define i82547EI_DEVICE_ID 0x1019

typedef struct __attribute__((packed)){
    uint32_t phys_buffer_addr_low;
    uint32_t phys_buffer_addr_high;
    uint16_t length;
    uint8_t checksum_offset;
    uint8_t command;
    uint8_t status;
    uint8_t checksum_start;
    uint16_t special;
}tx_descriptor_t;

typedef struct __attribute__((packed)){
    uint32_t phys_buffer_addr_low;
    uint32_t phys_buffer_addr_high;
    uint16_t length;
    uint16_t reserved1;
    uint8_t status;
    uint8_t errors;
    uint16_t reserved2;
}rx_descriptor_t;



void initI8254x(pci_dev_t* eth_adapter);
size_t i8254xSend(void* data, size_t length);
void i8254xHandler(intr_regs_t* regs);
