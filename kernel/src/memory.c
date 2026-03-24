#include "../include/memory.h"

static uint32_t page_frame_min;	// the first frame number that is safe to allocate (everything below is kernel/modules)
static uint32_t page_frame_max;	// the last usable frame number based on how much RAM the machine has.
static uint32_t total_alloc;	// total allocated memory

// has to be done as a dynamic bit array
uint8_t physical_mem_bitmap[NUM_PAGE_FRAMES / 8];	// each bit tracks one 4KB physical page frame (1 allocated, 0 free)

// This pre-allocates a pool of 256 page directories, each 4KB (1024 × 4-byte entries).
static uint32_t page_dirs[NUM_PAGE_DIRS][1024] __attribute__((aligned(4096)));
static uint8_t page_dirs_used[NUM_PAGE_DIRS]; // simple boolean array tracking which slots in the pool are taken
int mem_num_virt_pages;

void init_memory(multiboot_info_struct* boot_info){
	mem_num_virt_pages = 0;
	/*
		+ 0xFFF: the next phys page
		& ~0xFFF: masks off the lower 12 bits -> start of the next 4KB phys page, safe to allocate
	*/
	uint32_t physical_alloc_start = ((uint32_t)&_kernel_end - KERNEL_START + 0xFFF) & ~0xFFF;
	uint32_t mem_high = boot_info->mem_upper * 1024;

	// clears the entry, marking that 4MB region as not present for null pointer dereferencing
	initial_page_dir[0] = 0;
	// making an access to 0x0 a page fault
	invalidate(0);

	// recursive virtual address to the page_dir
	initial_page_dir[1023] = ((uint32_t) initial_page_dir - KERNEL_START) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;

	invalidate(0xFFFFF000);
	
	pmm_init(physical_alloc_start, mem_high);
	
	bios_term_print("DBG: Memory initialization success\n");
	return;
}

// flushes the Translation Lookaside Buffer (in CPU) for that address
void invalidate(uint32_t virt_addr){
	asm volatile("invlpg (%0)" :: "r"(virt_addr));
	return;
}

void pmm_init(uint32_t mem_low, uint32_t mem_high){
	page_frame_min = CEIL_DIV(mem_low, 0x1000);
	page_frame_max = mem_high / 0x1000;
	total_alloc = 0;

	memset(physical_mem_bitmap, 0, sizeof(physical_mem_bitmap));
	memset(page_dirs, 0, 0x1000 * NUM_PAGE_DIRS);
	memset(page_dirs_used, 0, NUM_PAGE_DIRS);
	return;
}

uint32_t pmm_alloc_page_frame(){
	uint32_t start = page_frame_min / 8 + ((page_frame_min & 7) != 0 ? 1 : 0);
	uint32_t end = page_frame_max / 8 - ((page_frame_max & 7) != 0 ? 1 : 0);

	for (uint32_t b = start; b < end; b++){
		uint8_t byte = physical_mem_bitmap[b];
		if (byte == 0xFF){
			continue;
		}
		
		for(uint32_t i = 0; i < 8; i++){
			uint8_t used = byte >> i & 1;
			if (!used){
				byte ^= (-1 ^ byte) & (1 << i);
				physical_mem_bitmap[b] = byte;
				total_alloc++;

				uint32_t addr = (b * 8 + i) + 0x1000;
				return addr;
			}
		}
	}

	return 0;
}

void mem_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags){
	uint32_t* prev_page_dir = 0;

	if(virt_addr >= KERNEL_START){
		prev_page_dir = mem_get_curr_page_dir();
		if(prev_page_dir != initial_page_dir){
			mem_change_curr_page_dir(initial_page_dir);
		}
	}

	uint32_t page_dir_index = virt_addr >> 22;
	uint32_t page_table_index = virt_addr >> 12 & 0x3FF;

	uint32_t* page_dir = REC_PAGE_DIR;
	uint32_t* page_table = REC_PAGE_TABLE(page_dir_index);

	if(!(page_dir[page_dir_index] & PAGE_FLAG_PRESENT)){
		uint32_t page_table_addr = pmm_alloc_page_frame();
		page_dir[page_dir_index] = page_table_addr | PAGE_FLAG_PRESENT |PAGE_FLAG_WRITE | PAGE_FLAG_OWNER | flags;
		invalidate(virt_addr);

		for(uint32_t i = 0; i < 1024; i++){
			page_table[i] = 0;
		}
	}

	page_table[page_table_index] = phys_addr | PAGE_FLAG_PRESENT | flags;
	mem_num_virt_pages++;
	invalidate(virt_addr);

	if (prev_page_dir != 0){
		sync_page_dirs();
		if (prev_page_dir != initial_page_dir){
			mem_change_curr_page_dir(prev_page_dir);
		}
	}
	return;
}

uint32_t* mem_get_curr_page_dir(){
	uint32_t page_dir;
	asm volatile("MOV %%cr3, %0": "=r"(page_dir));
	return (uint32_t*)(page_dir + KERNEL_START);
}

void mem_change_curr_page_dir(uint32_t* page_dir){
	page_dir = (uint32_t*)(((uint32_t)page_dir) - KERNEL_START);
	asm volatile("MOV %0, %%eax \n MOV %%eax, %%cr3 \n" :: "m"(page_dir));
	return;
}

void sync_page_dirs(){
	for(int i = 0; i < NUM_PAGE_DIRS; i++){
		if(page_dirs_used[i]){
			uint32_t* page_dir = page_dirs[i];

			for(int j = 768; j < 1023; j++){
				page_dir[j] = initial_page_dir[j] & ~PAGE_FLAG_OWNER;
			}
		}
	}
	return;
}
