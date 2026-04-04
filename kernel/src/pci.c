#include "pci.h"

/*
IMPROVEMENTS:
    - automatic detection, wether system supports pci:
        - with bios systems: "int 0x1A, AX=0xB101"
        - with uefi: does "PCI bus protocoll" exist
        - maybe ACPI tables
    - static device map maybe slow
    - not sure if i always need full specs


    - if at a later point i try to make a big endian system: pci is always little endian
*/

/*
NOTES:
    - Standard PCI Bus: 256 buses with each 32 devices with each 8 functions
*/

static kll_node* g_pci_dev_kll = NULL; // Dynamic list of PCI devices
static uint8_t g_bus_scan[32]; // already scanned bus bitmap: 256 buses = 32 bytes

static int pciScanDevice(uint32_t bus, uint32_t slot, uint32_t func);

/*
Tells the PCI device to send out its specifications
*/
static uint32_t pciGetSpecDWord(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset){
    /*
    Configuration address:
    Bit 31 	    Bits 30-24 	Bits 23-16 	Bits 15-11 	    Bits 10-8 	    Bits 7-0
    Enable Bit 	Reserved 	Bus Number 	Device Number 	Function Number Register Offset
    */
    uint32_t address = (0x80000000 | (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC));
    outPortL(PCI_CONFIG_ADDRESS, address);

    return inPortL(PCI_CONFIG_DATA);
}

/*
Brute force get and set of the devices specs
*/
static void pciParseSpec(pci_dev_spec_t* dev_spec, uint32_t bus, uint32_t slot, uint32_t func){
    uint32_t temp = pciGetSpecDWord(bus, slot, func, 0x0);
    dev_spec->vendor_id = (uint16_t)(temp & 0xFFFF);
    dev_spec->device_id = (uint16_t)((temp >> 16) & 0xFFFF);

    temp = pciGetSpecDWord(bus, slot, func, 0x4);
    dev_spec->command = (uint16_t)(temp & 0xFFFF);
    dev_spec->status = (uint16_t)((temp >> 16) & 0xFFFF);

    temp = pciGetSpecDWord(bus, slot, func, 0x8);
    dev_spec->rev_id = (uint8_t)(temp & 0xFF);
    dev_spec->prog_if = (uint8_t)((temp >> 8) & 0xFF);
    dev_spec->subclass = (uint8_t)((temp >> 16) & 0xFF);
    dev_spec->class = (uint8_t)((temp >> 24) & 0xFF);

    temp = pciGetSpecDWord(bus, slot, func, 0xC);
    dev_spec->cache_line_size = (uint8_t)(temp & 0xFF);
    dev_spec->lat_timer = (uint8_t)((temp >> 8) & 0xFF);
    dev_spec->header_type = (uint8_t)((temp >> 16) & 0x7F);
    dev_spec->mult_func = (uint8_t)((temp >> 23) & 0x1);
    dev_spec->bist = (uint8_t)((temp >> 24) & 0xFF);

    switch (dev_spec->header_type){
    case 0x0:
        dev_spec->gen_dev.BADR0 = pciGetSpecDWord(bus, slot, func, 0x10);
        dev_spec->gen_dev.BADR1 = pciGetSpecDWord(bus, slot, func, 0x14);
        dev_spec->gen_dev.BADR2 = pciGetSpecDWord(bus, slot, func, 0x18);
        dev_spec->gen_dev.BADR3 = pciGetSpecDWord(bus, slot, func, 0x1C);
        dev_spec->gen_dev.BADR4 = pciGetSpecDWord(bus, slot, func, 0x20);
        dev_spec->gen_dev.BADR5 = pciGetSpecDWord(bus, slot, func, 0x24);
        dev_spec->gen_dev.cardbus_cis_pointer = pciGetSpecDWord(bus, slot, func, 0x28);

        temp = pciGetSpecDWord(bus, slot, func, 0x2C);
        dev_spec->gen_dev.subsys_vendor_id = (uint16_t)(temp & 0xFFFF);
        dev_spec->gen_dev.subsys_id = (uint16_t)((temp >> 16) & 0xFFFF);

        dev_spec->gen_dev.exp_rom_base_adr = pciGetSpecDWord(bus, slot, func, 0x30);
        dev_spec->gen_dev.cap_pointer = (uint8_t)(pciGetSpecDWord(bus, slot, func, 0x34) & 0xFF);

        temp = pciGetSpecDWord(bus, slot, func, 0x3C);
        dev_spec->gen_dev.itr_line = (uint8_t)(temp & 0xFF);
        dev_spec->gen_dev.itr_pin = (uint8_t)((temp >> 8) & 0xFF);
        dev_spec->gen_dev.min_grant = (uint8_t)((temp >> 16) & 0xFF);
        dev_spec->gen_dev.max_lat = (uint8_t)((temp >> 24) & 0xFF);
        break;

    case 0x1:
        dev_spec->pci_to_pci_brg.BADR0 = pciGetSpecDWord(bus, slot, func, 0x10);
        dev_spec->pci_to_pci_brg.BADR1 = pciGetSpecDWord(bus, slot, func, 0x14);

        temp = pciGetSpecDWord(bus, slot, func, 0x18);
        dev_spec->pci_to_pci_brg.prim_bus_num = (uint8_t)(temp & 0xFF);
        dev_spec->pci_to_pci_brg.sec_bus_num = (uint8_t)((temp >> 8) & 0xFF);
        dev_spec->pci_to_pci_brg.sub_bus_num = (uint8_t)((temp >> 16) & 0xFF);
        dev_spec->pci_to_pci_brg.sec_lat_timer = (uint8_t)((temp >> 24) & 0xFF);

        temp = pciGetSpecDWord(bus, slot, func, 0x1C);
        dev_spec->pci_to_pci_brg.io_base = (uint8_t)(temp & 0xFF);
        dev_spec->pci_to_pci_brg.io_limit = (uint8_t)((temp >> 8) & 0xFF);
        dev_spec->pci_to_pci_brg.sec_status = (uint16_t)((temp >> 16) & 0xFFFF);
        
        temp = pciGetSpecDWord(bus, slot, func, 0x20);
        dev_spec->pci_to_pci_brg.mem_base = (uint16_t)(temp & 0xFFFF);
        dev_spec->pci_to_pci_brg.mem_limit = (uint16_t)((temp >> 16) & 0xFFFF);

        temp = pciGetSpecDWord(bus, slot, func, 0x24);
        dev_spec->pci_to_pci_brg.pref_mem_base = (uint16_t)(temp & 0xFFFF);
        dev_spec->pci_to_pci_brg.pref_mem_limit = (uint16_t)((temp >> 16) & 0xFFFF);

        dev_spec->pci_to_pci_brg.pref_mem_base_upper = pciGetSpecDWord(bus, slot, func, 0x28);
        dev_spec->pci_to_pci_brg.pref_mem_limit_upper = pciGetSpecDWord(bus, slot, func, 0x2C);

        temp = pciGetSpecDWord(bus, slot, func, 0x30);
        dev_spec->pci_to_pci_brg.io_base_upper = (uint16_t)(temp & 0xFFFF);
        dev_spec->pci_to_pci_brg.io_limit_upper = (uint16_t)((temp >> 16) & 0xFFFF);

        dev_spec->pci_to_pci_brg.cap_pointer = (uint8_t)(pciGetSpecDWord(bus, slot, func, 0x34) & 0xFF);
        dev_spec->pci_to_pci_brg.exp_rom_base_adr = pciGetSpecDWord(bus, slot, func, 0x38);

        temp = pciGetSpecDWord(bus, slot, func, 0x3C);
        dev_spec->pci_to_pci_brg.itr_line = (uint8_t)(temp & 0xFF);
        dev_spec->pci_to_pci_brg.itr_pin = (uint8_t)((temp >> 8) & 0xFF);
        dev_spec->pci_to_pci_brg.brg_ctl = (uint16_t)((temp >> 16) & 0xFFFF);
        break;

    case 0x2:
        dev_spec->pci_to_cardbus_brg.cardbus_soc_adr = pciGetSpecDWord(bus, slot, func, 0x10);

        temp = pciGetSpecDWord(bus, slot, func, 0x14);
        dev_spec->pci_to_cardbus_brg.offset_cap_list = (uint8_t)(temp & 0xFF);
        dev_spec->pci_to_cardbus_brg.sec_status = (uint16_t)((temp >> 16) & 0xFFFF);

        temp = pciGetSpecDWord(bus, slot, func, 0x18);
        dev_spec->pci_to_cardbus_brg.pci_bus_num = (uint8_t)(temp & 0xFF);
        dev_spec->pci_to_cardbus_brg.cardbus_num = (uint8_t)((temp >> 8) & 0xFF);
        dev_spec->pci_to_cardbus_brg.sub_bus_num = (uint8_t)((temp >> 16) & 0xFF);
        dev_spec->pci_to_cardbus_brg.cardbus_lat_timer = (uint8_t)((temp >> 24) & 0xFF);

        dev_spec->pci_to_cardbus_brg.mem_base_addr_0 = pciGetSpecDWord(bus, slot, func, 0x1C);
        dev_spec->pci_to_cardbus_brg.mem_lim_0 = pciGetSpecDWord(bus, slot, func, 0x20);
        dev_spec->pci_to_cardbus_brg.mem_base_addr_1 = pciGetSpecDWord(bus, slot, func, 0x24);
        dev_spec->pci_to_cardbus_brg.mem_lim_1 = pciGetSpecDWord(bus, slot, func, 0x28);
        dev_spec->pci_to_cardbus_brg.io_base_addr_0 = pciGetSpecDWord(bus, slot, func, 0x2C);
        dev_spec->pci_to_cardbus_brg.io_lim_0 = pciGetSpecDWord(bus, slot, func, 0x30);
        dev_spec->pci_to_cardbus_brg.io_base_addr_1 = pciGetSpecDWord(bus, slot, func, 0x34);
        dev_spec->pci_to_cardbus_brg.io_lim_1 = pciGetSpecDWord(bus, slot, func, 0x38);

        temp = pciGetSpecDWord(bus, slot, func, 0x3C);
        dev_spec->pci_to_cardbus_brg.itr_line = (uint8_t)(temp & 0xFF);
        dev_spec->pci_to_cardbus_brg.itr_pin = (uint8_t)((temp >> 8) & 0xFF);
        dev_spec->pci_to_cardbus_brg.brg_ctl = (uint16_t)((temp >> 16) & 0xFFFF);

        temp = pciGetSpecDWord(bus, slot, func, 0x40);
        dev_spec->pci_to_cardbus_brg.subsys_dev_id = (uint16_t)(temp & 0xFFFF);
        dev_spec->pci_to_cardbus_brg.subsys_vendor_id = (uint16_t)((temp >> 16) & 0xFFFF);

        dev_spec->pci_to_cardbus_brg.legacy_mode_base_adr = pciGetSpecDWord(bus, slot, func, 0x44);
        break;
    }

    return;
}

/*

*/
static void pciScanBus(uint32_t bus) {
    // Check if already scanned, if not mark as scanned
    if (g_bus_scan[bus / 8] & (1 << (bus % 8)))
        return;

    g_bus_scan[bus / 8] |= (1 << (bus % 8));
    
    for (uint32_t slot = 0; slot < 32; slot++) {
        if (pciScanDevice(bus, slot, 0) == 1)
            continue;

        // Check multi-function bit on function 0
        uint32_t header = pciGetSpecDWord(bus, slot, 0, 0x0C);
        if ((header >> 23) & 0x1) {
            for (uint32_t func = 1; func < 8; func++) {
                pciScanDevice(bus, slot, func);
            }
        }
    }

    return;
}

/*

*/
static int pciScanDevice(uint32_t bus, uint32_t slot, uint32_t func) {
    /*
    If there the device is not existent, the pci host controller returns all 1
    */
    uint32_t reg0 = pciGetSpecDWord(bus, slot, func, 0x0);
    if (reg0 == 0xFFFFFFFF)
        return 1;

    // Allocate and link into the list
    pci_dev_t* pci_dev = (pci_dev_t*)kmalloc(sizeof(pci_dev_t));
    if (pci_dev == NULL){
        biosTermPrintf("ERR: Kmalloc\n");
        return 1;
    }
    g_pci_dev_kll = kllAddNode(g_pci_dev_kll, pci_dev);

    // Fill in the specs
    pci_dev->bus = bus;
    pci_dev->slot = slot;
    pci_dev->func = func;
    pciParseSpec(&(pci_dev->dev_spec), bus, slot, func);

    // If it's a PCI-to-PCI bridge, recurse into the secondary bus
    if (pci_dev->dev_spec.header_type == 0x1) {
        uint32_t sec_bus_num = pci_dev->dev_spec.pci_to_pci_brg.sec_bus_num;
        pciScanBus(sec_bus_num);
    }
    return 0;
}

void initPCI() {
    // Check if host controller itself is multi-function
    uint32_t header = pciGetSpecDWord(0, 0, 0, 0x0C);
    if ((header >> 23) & 0x1) {
        // Multiple PCI host controllers — each function is a separate bus
        for (uint32_t func = 0; func < 8; func++) {
            if (pciGetSpecDWord(0, 0, func, 0x0) != 0xFFFFFFFF) {
                pciScanBus(func);
            }
        }
    }
    else
        pciScanBus(0);

    biosTermPrintf("DBG: PCI init success. Devices: %d\n", (int)kllGetLength(g_pci_dev_kll));
    return;
}

void enumeratePCI(){
    int dev_count = kllGetLength(g_pci_dev_kll);
    pci_dev_t* pci_dev;

    for (int i = 0; i < dev_count; i++){
        pci_dev = kllGetData(g_pci_dev_kll, i);
        biosTermPrintf("PIC Dev %d:\tBus: %d\tSlot: %d\tFunction: %d\tClass: %d\tSubclass: %d\n",
            (i + 1), pci_dev->bus, pci_dev->slot, pci_dev->func, pci_dev->dev_spec.class, pci_dev->dev_spec.subclass);
    }

    return;
}
