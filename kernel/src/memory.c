#include "../include/memory.h"

void init_memory(multiboot_info_struct* boot_info){
	for (int i = 0; i < boot_info->mmap_lenght; i += sizeof(multiboot_mmap_entry_struct)){
		multiboot_mmap_entry_struct* multiboot_mmap_entry = 
			(multiboot_mmap_entry_struct*)(boot_info->mmap_addr + i);
		
	}
	return;
}


