#pragma once

#include <stdint.h>
#include <stddef.h>
#include "kmalloc.h"
#include "kll.h"
#include "bios_term.h"
#include "util.h"

#define PCI_CLASS_NET_CTL 0x02
#define PCI_SUBCLASS_ETH_CTL 0x00

#define PCI_MEMORY_SPACE_ENABLE (1 << 1)
#define PCI_BUS_MASTER_ENABLE (1 << 2)

typedef struct pci_dev_t{
    uint8_t bus;
    uint8_t slot;
    uint8_t func;

    uint16_t vendor_id;
    uint16_t device_id;

    uint8_t class;
    uint8_t subclass;
    uint8_t prog_if;
    uint8_t rev_id;

    uint8_t header_type; // 0x0 general device, 0x1 pci-to-pci-bridge, 0x2 pci-to-cardbus-bridge
    uint8_t mult_func; // Multiple functions: 1 yes, 0 no
} pci_dev_t;

void initPCI();
uint32_t pciReadDWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pciWriteDWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t data);
pci_dev_t* pciFindDevType(uint8_t class, uint8_t subclass, uint8_t prog_if);
pci_dev_t* pciFindDevID(uint16_t vendor_id, uint16_t device_id);
