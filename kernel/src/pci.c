#include "../include/pci.h"

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lBus  = (uint32_t)bus;
    uint32_t lSlot = (uint32_t)slot;
    uint32_t lFunc = (uint32_t)func;
    uint16_t ret = 0;
  
    // Create configuration address as per Figure 1
    address = (uint32_t)((lBus << 16) | (lSlot << 11) |
              (lFunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
  
    // Write out the address
    outPortL(CONFIG_ADDRESS, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    ret = (uint16_t)((inPortL(CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
    return ret;
}