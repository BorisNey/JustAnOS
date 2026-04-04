#pragma once

typedef struct __attribute__((packed)){
	uint32_t tabsize;
	uint32_t strsize;
	uint32_t addr;
	uint32_t reserved;
}mb_aout_symbol_table_t;

typedef struct __attribute__((packed)){
	uint32_t num;
	uint32_t size;
	uint32_t addr;
	uint32_t shndx;
}mb_elf_section_header_table_t;

typedef struct __attribute__((packed)){
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint32_t boot_device;
	
	uint32_t cmdline;
	uint32_t mods_count;
	uint32_t mods_addr;		// multiboot modules array

	union{
		mb_aout_symbol_table_t aout_sym;
		mb_elf_section_header_table_t elf_sec;
	}u;
	
	uint32_t mmap_lenght;
	uint32_t mmap_addr;
	
	uint32_t drives_lenght;
	uint32_t drives_addr;

	uint32_t config_table;
	uint32_t boot_loader_name;

	uint32_t apm_table;

	uint32_t vbe_control_info;
	uint32_t vbe_mode_info;
	uint32_t vbe_interface_seg;
	uint32_t vbe_interface_off;
	uint32_t vbe_interface_len;
}mb_info_t;

typedef struct __attribute__((packed)){
	uint32_t size;
	uint32_t addr_low;
	uint32_t addr_high;
	uint32_t len_low;
	uint32_t len_high;

#define MULTIBOOT_MEMORY_AVAILABLE			1
#define MULTIBOOT_MEMORY_RESERVED			2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE	3
#define MULTIBOOT_MEMORY_NVS				4
#define MULTIBOOT_MEMORY_BADRAM				5

	uint32_t type;
}mb_mmap_entry_t;
