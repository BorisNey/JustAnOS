#include "../include/pci.h"

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

static pci_dev_spec_t* g_dev_map[256][32][8]; // Standard PCI Bus: 256 buses with each 32 devices with each 8 functions

void initPCI(){
    int dev_number = 0;
    // Go through each device on each bus and map its full spec
    for (uint32_t bus = 0; bus < 256; bus++){
        for (uint32_t dev = 0; dev < 32; dev++){
            g_dev_map[bus][dev][0] = pciGetFullDevSpec(bus, dev, 0);
            if (g_dev_map[bus][dev][0] != NULL){
                dev_number++;
                if (g_dev_map[bus][dev][0]->mult_func == 0x1){
                    for (uint32_t func = 1; func < 8; func++){
                        g_dev_map[bus][dev][func] = pciGetFullDevSpec(bus, dev, func);
                    }
                }
            }
        }
    }
    biosTermPrintf("DBG: PCI init success. PCI Devices: %d\n", dev_number);
    return;
}

/*
Tells the PCI device to send out its specifications
*/
uint32_t pciGetSpecDWord(uint32_t bus, uint32_t dev, uint32_t func, uint32_t offset){
    /*
    Configuration address:
    Bit 31 	    Bits 30-24 	Bits 23-16 	Bits 15-11 	    Bits 10-8 	    Bits 7-0
    Enable Bit 	Reserved 	Bus Number 	Device Number 	Function Number Register Offset
    */
    uint32_t address = (0x80000000 | (bus << 16) | (dev << 11) | (func << 8) | (offset & 0xFC));
  
    // Write out the address
    outPortL(CONFIG_ADDRESS, address);

    // Read in the data
    return inPortL(CONFIG_DATA);
}

/*
If the specified device is existent, kmallocs a struct with all its specifications
!! Remember to kfree later !!
*/
pci_dev_spec_t* pciGetFullDevSpec(uint32_t bus, uint32_t dev, uint32_t func) {
    uint32_t temp;
    /*
    Try and read the first configuration register
    host bridge returns 0xFFFF, if its a non-existent device.
    */
    if ((temp = pciGetSpecDWord(bus, dev, func, 0x0)) == 0xFFFFFFFF) {
        return NULL;
    }
    pci_dev_spec_t* dev_spec = (pci_dev_spec_t*)kmalloc(sizeof(pci_dev_spec_t));
    if ( dev_spec == NULL ){
        biosTermPrintf("ERR: Kmalloc\n");
        return NULL;
    }
    dev_spec->vendor_id = (uint16_t)(temp & 0xFFFF);
    dev_spec->device_id = (uint16_t)((temp >> 16) & 0xFFFF);

    temp = pciGetSpecDWord(bus, dev, func, 0x4);
    dev_spec->command = (uint16_t)(temp & 0xFFFF);
    dev_spec->status = (uint16_t)((temp >> 16) & 0xFFFF);

    temp = pciGetSpecDWord(bus, dev, func, 0x8);
    dev_spec->rev_id = (uint8_t)(temp & 0xFF);
    dev_spec->prog_if = (uint8_t)((temp >> 8) & 0xFF);
    dev_spec->subclass = (uint8_t)((temp >> 16) & 0xFF);
    dev_spec->class = (uint8_t)((temp >> 24) & 0xFF);

    temp = pciGetSpecDWord(bus, dev, func, 0xC);
    dev_spec->cache_line_size = (uint8_t)(temp & 0xFF);
    dev_spec->lat_timer = (uint8_t)((temp >> 8) & 0xFF);
    dev_spec->header_type = (uint8_t)((temp >> 16) & 0x7F);
    dev_spec->mult_func = (uint8_t)((temp >> 23) & 0x1);
    dev_spec->bist = (uint8_t)((temp >> 24) & 0xFF);

    switch (dev_spec->header_type){
    case 0x0:
        dev_spec->gen_dev.BADR0 = pciGetSpecDWord(bus, dev, func, 0x10);
        dev_spec->gen_dev.BADR1 = pciGetSpecDWord(bus, dev, func, 0x14);
        dev_spec->gen_dev.BADR2 = pciGetSpecDWord(bus, dev, func, 0x18);
        dev_spec->gen_dev.BADR3 = pciGetSpecDWord(bus, dev, func, 0x1C);
        dev_spec->gen_dev.BADR4 = pciGetSpecDWord(bus, dev, func, 0x20);
        dev_spec->gen_dev.BADR5 = pciGetSpecDWord(bus, dev, func, 0x24);
        dev_spec->gen_dev.cardbus_cis_pointer = pciGetSpecDWord(bus, dev, func, 0x28);

        temp = pciGetSpecDWord(bus, dev, func, 0x2C);
        dev_spec->gen_dev.subsys_vendor_id = (uint16_t)(temp & 0xFFFF);
        dev_spec->gen_dev.subsys_id = (uint16_t)((temp >> 16) & 0xFFFF);

        dev_spec->gen_dev.exp_rom_base_adr = pciGetSpecDWord(bus, dev, func, 0x30);
        dev_spec->gen_dev.cap_pointer = (uint8_t)(pciGetSpecDWord(bus, dev, func, 0x34) & 0xFF);

        temp = pciGetSpecDWord(bus, dev, func, 0x3C);
        dev_spec->gen_dev.itr_line = (uint8_t)(temp & 0xFF);
        dev_spec->gen_dev.itr_pin = (uint8_t)((temp >> 8) & 0xFF);
        dev_spec->gen_dev.min_grant = (uint8_t)((temp >> 16) & 0xFF);
        dev_spec->gen_dev.max_lat = (uint8_t)((temp >> 24) & 0xFF);
        break;

    case 0x1:
        dev_spec->pci_to_pci_brg.BADR0 = pciGetSpecDWord(bus, dev, func, 0x10);
        dev_spec->pci_to_pci_brg.BADR1 = pciGetSpecDWord(bus, dev, func, 0x14);

        temp = pciGetSpecDWord(bus, dev, func, 0x18);
        dev_spec->pci_to_pci_brg.prim_bus_num = (uint8_t)(temp & 0xFF);
        dev_spec->pci_to_pci_brg.sec_bus_num = (uint8_t)((temp >> 8) & 0xFF);
        dev_spec->pci_to_pci_brg.sub_bus_num = (uint8_t)((temp >> 16) & 0xFF);
        dev_spec->pci_to_pci_brg.sec_lat_timer = (uint8_t)((temp >> 24) & 0xFF);

        temp = pciGetSpecDWord(bus, dev, func, 0x1C);
        dev_spec->pci_to_pci_brg.io_base = (uint8_t)(temp & 0xFF);
        dev_spec->pci_to_pci_brg.io_limit = (uint8_t)((temp >> 8) & 0xFF);
        dev_spec->pci_to_pci_brg.sec_status = (uint16_t)((temp >> 16) & 0xFFFF);
        
        temp = pciGetSpecDWord(bus, dev, func, 0x20);
        dev_spec->pci_to_pci_brg.mem_base = (uint16_t)(temp & 0xFFFF);
        dev_spec->pci_to_pci_brg.mem_limit = (uint16_t)((temp >> 16) & 0xFFFF);

        temp = pciGetSpecDWord(bus, dev, func, 0x24);
        dev_spec->pci_to_pci_brg.pref_mem_base = (uint16_t)(temp & 0xFFFF);
        dev_spec->pci_to_pci_brg.pref_mem_limit = (uint16_t)((temp >> 16) & 0xFFFF);

        dev_spec->pci_to_pci_brg.pref_mem_base_upper = pciGetSpecDWord(bus, dev, func, 0x28);
        dev_spec->pci_to_pci_brg.pref_mem_limit_upper = pciGetSpecDWord(bus, dev, func, 0x2C);

        temp = pciGetSpecDWord(bus, dev, func, 0x30);
        dev_spec->pci_to_pci_brg.io_base_upper = (uint16_t)(temp & 0xFFFF);
        dev_spec->pci_to_pci_brg.io_limit_upper = (uint16_t)((temp >> 16) & 0xFFFF);

        dev_spec->pci_to_pci_brg.cap_pointer = (uint8_t)(pciGetSpecDWord(bus, dev, func, 0x34) & 0xFF);
        dev_spec->pci_to_pci_brg.exp_rom_base_adr = pciGetSpecDWord(bus, dev, func, 0x38);

        temp = pciGetSpecDWord(bus, dev, func, 0x3C);
        dev_spec->pci_to_pci_brg.itr_line = (uint8_t)(temp & 0xFF);
        dev_spec->pci_to_pci_brg.itr_pin = (uint8_t)((temp >> 8) & 0xFF);
        dev_spec->pci_to_pci_brg.brg_ctl = (uint16_t)((temp >> 16) & 0xFFFF);
        break;

    case 0x2:
        dev_spec->pci_to_cardbus_brg.cardbus_soc_adr = pciGetSpecDWord(bus, dev, func, 0x10);

        temp = pciGetSpecDWord(bus, dev, func, 0x14);
        dev_spec->pci_to_cardbus_brg.offset_cap_list = (uint8_t)(temp & 0xFF);
        dev_spec->pci_to_cardbus_brg.sec_status = (uint16_t)((temp >> 16) & 0xFFFF);

        temp = pciGetSpecDWord(bus, dev, func, 0x18);
        dev_spec->pci_to_cardbus_brg.pci_bus_num = (uint8_t)(temp & 0xFF);
        dev_spec->pci_to_cardbus_brg.cardbus_num = (uint8_t)((temp >> 8) & 0xFF);
        dev_spec->pci_to_cardbus_brg.sub_bus_num = (uint8_t)((temp >> 16) & 0xFF);
        dev_spec->pci_to_cardbus_brg.cardbus_lat_timer = (uint8_t)((temp >> 24) & 0xFF);

        dev_spec->pci_to_cardbus_brg.mem_base_addr_0 = pciGetSpecDWord(bus, dev, func, 0x1C);
        dev_spec->pci_to_cardbus_brg.mem_lim_0 = pciGetSpecDWord(bus, dev, func, 0x20);
        dev_spec->pci_to_cardbus_brg.mem_base_addr_1 = pciGetSpecDWord(bus, dev, func, 0x24);
        dev_spec->pci_to_cardbus_brg.mem_lim_1 = pciGetSpecDWord(bus, dev, func, 0x28);
        dev_spec->pci_to_cardbus_brg.io_base_addr_0 = pciGetSpecDWord(bus, dev, func, 0x2C);
        dev_spec->pci_to_cardbus_brg.io_lim_0 = pciGetSpecDWord(bus, dev, func, 0x30);
        dev_spec->pci_to_cardbus_brg.io_base_addr_1 = pciGetSpecDWord(bus, dev, func, 0x34);
        dev_spec->pci_to_cardbus_brg.io_lim_1 = pciGetSpecDWord(bus, dev, func, 0x38);

        temp = pciGetSpecDWord(bus, dev, func, 0x3C);
        dev_spec->pci_to_cardbus_brg.itr_line = (uint8_t)(temp & 0xFF);
        dev_spec->pci_to_cardbus_brg.itr_pin = (uint8_t)((temp >> 8) & 0xFF);
        dev_spec->pci_to_cardbus_brg.brg_ctl = (uint16_t)((temp >> 16) & 0xFFFF);

        temp = pciGetSpecDWord(bus, dev, func, 0x40);
        dev_spec->pci_to_cardbus_brg.subsys_dev_id = (uint16_t)(temp & 0xFFFF);
        dev_spec->pci_to_cardbus_brg.subsys_vendor_id = (uint16_t)((temp >> 16) & 0xFFFF);

        dev_spec->pci_to_cardbus_brg.legacy_mode_base_adr = pciGetSpecDWord(bus, dev, func, 0x44);
        break;
    }

    return dev_spec;
}
