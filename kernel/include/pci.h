#pragma once

#include <stdint.h>
#include <stddef.h>
#include "kmalloc.h"
#include "kll.h"
#include "bios_term.h"
#include "util.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

typedef struct{
    uint32_t BADR0;
    uint32_t BADR1;
    uint32_t BADR2;
    uint32_t BADR3;
    uint32_t BADR4;
    uint32_t BADR5;
    uint32_t cardbus_cis_pointer;
    
    uint16_t subsys_vendor_id;
    uint16_t subsys_id;

    uint32_t exp_rom_base_adr;

    uint8_t cap_pointer;
    // 24 Bit Reserved
    // 32 Bit Reserved

    uint8_t itr_line;
    uint8_t itr_pin;
    uint8_t min_grant;
    uint8_t max_lat;
} gen_dev_t;

typedef struct{
    uint32_t BADR0;
    uint32_t BADR1;

    uint8_t prim_bus_num;
    uint8_t sec_bus_num;
    uint8_t sub_bus_num;
    uint8_t sec_lat_timer;

    uint8_t io_base;
    uint8_t io_limit;
    uint16_t sec_status;

    uint16_t mem_base;
    uint16_t mem_limit;

    uint16_t pref_mem_base;
    uint16_t pref_mem_limit;

    uint32_t pref_mem_base_upper;
    uint32_t pref_mem_limit_upper;
    
    uint16_t io_base_upper;
    uint16_t io_limit_upper;

    uint8_t cap_pointer;
    // 24 Bit Reserved

    uint32_t exp_rom_base_adr;

    uint8_t itr_line;
    uint8_t itr_pin;
    uint16_t brg_ctl;
} pci_to_pci_brg_t;

typedef struct{
    uint32_t cardbus_soc_adr;

    uint8_t offset_cap_list;
    // 8 Bit Reserved
    uint16_t sec_status;

    uint8_t pci_bus_num;
    uint8_t cardbus_num;
    uint8_t sub_bus_num;
    uint8_t cardbus_lat_timer;

    uint32_t mem_base_addr_0;
    uint32_t mem_lim_0;
    uint32_t mem_base_addr_1;
    uint32_t mem_lim_1;
    uint32_t io_base_addr_0;
    uint32_t io_lim_0;
    uint32_t io_base_addr_1;
    uint32_t io_lim_1;

    uint8_t itr_line;
    uint8_t itr_pin;
    uint16_t brg_ctl;

    uint16_t subsys_dev_id;
    uint16_t subsys_vendor_id;

    uint32_t legacy_mode_base_adr;
} pci_to_cardbus_brg_t;

typedef struct{
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;

    uint8_t rev_id;
    uint8_t prog_if;
    uint8_t subclass;
    uint8_t class;

    uint8_t cache_line_size;
    uint8_t lat_timer;
    uint8_t header_type; // 0x0 general device, 0x1 pci-to-pci-bridge, 0x2 pci-to-cardbus-bridge
    uint8_t mult_func; // Multiple functions: 1 yes, 0 no
    uint8_t bist;

    union {
        gen_dev_t gen_dev;
        pci_to_pci_brg_t pci_to_pci_brg;
        pci_to_cardbus_brg_t pci_to_cardbus_brg;
    };

} pci_dev_spec_t;

typedef struct {
    uint32_t bus;
    uint32_t slot;
    uint32_t func;
    pci_dev_spec_t dev_spec;
} pci_dev_t;

uint32_t pciGetSpecDWord(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset);
void initPCI();
pci_dev_t* pciFindDevType(uint8_t class, uint8_t subclass, uint8_t prog_if);
pci_dev_t* pciFindDevID(uint16_t device_id, uint16_t vendor_id);
