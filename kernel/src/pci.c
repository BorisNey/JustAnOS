#include "pci.h"

/*
IMPROVEMENTS:
    - automatic detection, wether system supports pci:
        - with bios systems: "int 0x1A, AX=0xB101"
        - with uefi: does "PCI bus protocoll" exist
        - maybe ACPI tables
    - definitly dont need full specs


    - if at a later point i try to make a big endian system: pci is always little endian
*/

/*
NOTES:
    - Standard PCI Bus: 256 buses with each 32 devices with each 8 functions
*/

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

static kll_node* g_pci_dev_kll = NULL; // Dynamic list of PCI devices
static uint8_t g_bus_scan[32]; // already scanned bus bitmap: 256 buses = 32 bytes

static int pciScanDevice(uint8_t bus, uint8_t slot, uint8_t func);

/*

*/
static void pciScanBus(uint8_t bus) {
    // Check if already scanned, if not mark as scanned
    if (g_bus_scan[bus / 8] & (1 << (bus % 8))){
        return;
    }
    g_bus_scan[bus / 8] |= (1 << (bus % 8));
    
    for (uint8_t slot = 0; slot < 32; slot++){
        if (pciScanDevice(bus, slot, 0) == 1){
            continue;
        }
        // Check multi-function bit on function 0
        uint32_t multi_func = (pciReadDWord(bus, slot, 0, 0x0C) >> 23) & 0x1;
        if (multi_func){
            for (uint8_t func = 1; func < 8; func++) {
                pciScanDevice(bus, slot, func);
            }
        }
    }

    return;
}

/*

*/
static int pciScanDevice(uint8_t bus, uint8_t slot, uint8_t func) {
    /*
    If there the device is not existent, the pci host controller returns all 1
    */
    uint32_t temp = pciReadDWord(bus, slot, func, 0x0);
    if (temp == 0xFFFFFFFF)
        return 1;

    // Allocate and link into the list
    pci_dev_t* dev = (pci_dev_t*)kmalloc(sizeof(pci_dev_t));
    if (dev == NULL){
        biosTermPrintf("ERR: Kmalloc\n");
        return 1;
    }
    g_pci_dev_kll = kllAddNode(g_pci_dev_kll, dev);

    // Fill in the specs
    dev->bus = bus;
    dev->slot = slot;
    dev->func = func;

    dev->vendor_id = (uint16_t)(temp & 0xFFFF);
    dev->device_id = (uint16_t)((temp >> 16) & 0xFFFF);

    temp = pciReadDWord(bus, slot, func, 0x8);
    dev->rev_id = (uint8_t)(temp & 0xFF);
    dev->prog_if = (uint8_t)((temp >> 8) & 0xFF);
    dev->subclass = (uint8_t)((temp >> 16) & 0xFF);
    dev->class = (uint8_t)((temp >> 24) & 0xFF);

    temp = pciReadDWord(bus, slot, func, 0xC);
    dev->header_type = (uint8_t)((temp >> 16) & 0x7F);
    dev->mult_func = (uint8_t)((temp >> 23) & 0x1);

    // If it's a PCI-to-PCI bridge, recurse into the secondary bus
    if (dev->header_type == 0x1) {
        uint8_t sec_bus_num = (uint8_t)((pciReadDWord(bus, slot, func, 0x18) >> 8) & 0xFF);
        pciScanBus(sec_bus_num);
    }
    return 0;
}

void initPCI() {
    // Check if host controller itself is multi-function
    uint32_t mult_func = (pciReadDWord(0, 0, 0, 0x0C) >> 23) & 0x1;
    if (mult_func){
        // Multiple PCI host controllers — each function is a separate bus
        for (uint32_t func = 0; func < 8; func++){
            if (pciReadDWord(0, 0, func, 0x0) != 0xFFFFFFFF) {
                pciScanBus(func);
            }
        }
    }
    else
        pciScanBus(0);

    biosTermPrintf("DBG: PCI init success. Devices: %d\n", (int)kllGetLength(g_pci_dev_kll));
    return;
}

/*
Tells the PCI device to send out its specifications
*/
uint32_t pciReadDWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset){
    /*
    Configuration address:
    Bit 31 	    Bits 30-24 	Bits 23-16 	Bits 15-11 	    Bits 10-8 	    Bits 7-0
    Enable Bit 	Reserved 	Bus Number 	Device Number 	Function Number Register Offset
    */
    uint32_t address = (0x80000000 | (((uint32_t)bus) << 16) | (((uint32_t)slot) << 11) | (((uint32_t)func) << 8) | (((uint32_t)offset) & 0xFC));
    outPortL(PCI_CONFIG_ADDRESS, address);

    return inPortL(PCI_CONFIG_DATA);
}

/*
Writes data to a PCI device. Keep in mind to read the value first and then changing the value accordingly. 
Example:
    uint32_t val = pciReadDWord(bus, slot, func, offset);
    val = (val & 0xFFFFFF00) | (uint8_t)new_byte;  // replace lowest byte
    pciWriteDWord(bus, slot, func, offset, val);
*/
void pciWriteDWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t data){
    uint32_t address = (0x80000000 | (((uint32_t)bus) << 16) | (((uint32_t)slot) << 11) | (((uint32_t)func) << 8) | (((uint32_t)offset) & 0xFC));
    outPortL(PCI_CONFIG_ADDRESS, address);

    outPortL(PCI_CONFIG_DATA, data);
    return;
}

/*
to ignore prog_if, set to 0xFF
*/
pci_dev_t* pciFindDevType(uint8_t class, uint8_t subclass, uint8_t prog_if){
    int dev_count = kllGetLength(g_pci_dev_kll);
    pci_dev_t* dev;

    for (int i = 0; i < dev_count; i++){
        dev = kllGetData(g_pci_dev_kll, i);
        if (dev->class == class && dev->subclass == subclass){
            if (prog_if == 0xFF){
                return dev;
            }
            else if (dev->prog_if == prog_if){
                return dev;
            }
        }
    }
    return NULL;
}

pci_dev_t* pciFindDevID(uint16_t vendor_id, uint16_t device_id){
    int dev_count = kllGetLength(g_pci_dev_kll);
    pci_dev_t* dev;

    for (int i = 0; i < dev_count; i++){
        dev = kllGetData(g_pci_dev_kll, i);
        if (dev->device_id == device_id && dev->vendor_id == vendor_id){
            return dev;
        }
    }
    return NULL;
}
