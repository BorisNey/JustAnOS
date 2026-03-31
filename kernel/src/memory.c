#include "../include/memory.h"

/*
TODO in General:
	- page fault handling:
		-  in idt 11
	- kmalloc function
	
*/

static uint32_t page_frame_min; // the first frame number that is safe to allocate (everything below is kernel/modules)
static uint32_t page_frame_max; // the last frame number based on how much RAM the machine has.
static uint32_t total_alloc_pages; // total allocated memory
uint8_t phys_mem_bitmap[NUM_PAGE_FRAMES / 8];  // each bit tracks one 4KB physical page frame (1 allocated, 0 free)

// TODO: needs to be dynamically allocated
static uint32_t page_dirs[NUM_PAGE_DIRS][1024] __attribute__((aligned(PAGE_SIZE))); // This pre-allocates a pool of 256 page directories, each 4KB (1024 × 4-byte entries).
static uint8_t page_dirs_used[NUM_PAGE_DIRS]; // simple boolean array tracking which slots in the pool are taken

int num_virt_pages; // Number of virtual pages

void init_memory(multiboot_info_struct* boot_info){
	num_virt_pages = 0;

	/*
	* Calculates the next free page after kernel, which is safe to allocat to
	* 	+ 0xFFF: the next phys page
	*	& ~0xFFF: masks off the lower 12 bits -> start of the next 4KB phys page
	*/
	uint32_t physical_alloc_start = ((uint32_t)&_kernel_end - KERNEL_START + 0xFFF) & ~0xFFF;

	/*
	* Calculates last accessible address
	*	* 1024, because boot_info->mem_upper is in KB
	*/
	uint32_t mem_high = boot_info->mem_upper * 1024;

	/*
	* Clears the entry
	*	- marking that 4MB region as not present for null pointer dereferencing
	*	- making an access to virt 0x0 results in a page fault
	*/
	kernel_page_dir[0] = 0;
	invalidate_tlb_entry(0);

	/*
	* Setting up recursive virtual address to the page_dir
	*	- invalidating updates the tlb entry
	*/
	kernel_page_dir[1023] = ((uint32_t) kernel_page_dir - KERNEL_START) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;
	invalidate_tlb_entry(0xFFFFF000);
	
	pmm_init(physical_alloc_start, mem_high);

	// TODO maybe: go through boot_info->mmap_addr + i while boot_info->mmap_lenght and make entries in physical bitmap
	
	bios_term_print("DBG: Memory initialization success\n");
	return;
}

/*
* Flushes the Translation Lookaside Buffer (in CPU) for that address
* 	e.g. for updating the address
*/
void invalidate_tlb_entry(uint32_t virt_addr){
	asm volatile("invlpg (%0)" :: "r"(virt_addr));
	return;
}

/*
* Initialization of the Physical Memory Management
*/
void pmm_init(uint32_t mem_low, uint32_t mem_high){
	page_frame_min = CEIL_DIV(mem_low, PAGE_SIZE);
	page_frame_max = mem_high / PAGE_SIZE;
	total_alloc_pages = 0;

	memset(phys_mem_bitmap, 0, sizeof(phys_mem_bitmap));
	memset(page_dirs, 0, PAGE_SIZE * NUM_PAGE_DIRS);
	memset(page_dirs_used, 0, NUM_PAGE_DIRS);
	return;
}


/*
* Finds a free pyhsical page frame, sets it as allocated and returns its physical address
*/
uint32_t pmm_alloc_page_frame(){
	uint32_t start = page_frame_min / 8 + ((page_frame_min & 7) != 0 ? 1 : 0);
	uint32_t end = page_frame_max / 8 - ((page_frame_max & 7) != 0 ? 1 : 0);

	for (uint32_t b = start; b < end; b++){
		/*
		* Goes thru every pyhsical page frame and checks if allocated
		*/
		uint8_t byte = phys_mem_bitmap[b];
		if (byte == 0xFF){
			continue;
		}
		
		/*
		* If there is a free page set it allocated and return its physical address
		*/
		for(uint32_t i = 0; i < 8; i++){
			uint8_t used = (byte >> i) & 0x01;
			if (!used){
				byte ^= (0xFF ^ byte) & (1 << i);
				phys_mem_bitmap[b] = byte;
				total_alloc_pages++;

				uint32_t addr = (b * 8 + i) + PAGE_SIZE;
				return addr;
			}
		}
	}

	// no free page frame found
	return 0;
}

/*
* 
*/
void map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags){
	/*
	* making sure that the current page directory is the right one
	*/
	uint32_t* prev_page_dir = 0;
	if(virt_addr >= KERNEL_START){
		prev_page_dir = get_page_dir_addr();
		if(prev_page_dir != kernel_page_dir){
			change_page_dir_addr(kernel_page_dir);
		}
	}

	/*
	* A 32-bit virtual address is split into three parts by the x86 MMU:
	*	Bits 31–22: index into the page directory (which of 1024 page tables to use)
	*	Bits 21–12: index into the page table (which of 1024 pages within that table)
	*	Bits 11–0: offset within the 4KB page
	*/
	uint32_t page_dir_index = virt_addr >> 22;
	uint32_t page_table_index = virt_addr >> 12 & 0x3FF;

	uint32_t* page_dir = REC_PAGE_DIR;
	uint32_t* page_table = REC_PAGE_TABLE(page_dir_index);

	/*
	* If the page_dir entry of the virtual address is not present, 
	* we have to allocate in physical space (with pmm) and
	* make an fresh (all 0) entry in our page directory
	*/
	if(!(page_dir[page_dir_index] & PAGE_FLAG_PRESENT)){
		uint32_t page_table_addr = pmm_alloc_page_frame();
		page_dir[page_dir_index] = page_table_addr | PAGE_FLAG_PRESENT |PAGE_FLAG_WRITE | PAGE_FLAG_OWNER | flags;
		invalidate_tlb_entry(virt_addr);
		
		for(uint32_t i = 0; i < 1024; i++){
			page_table[i] = 0;
		}
	}

	/*
	* Actually mapping the virtual to the physical address
	*/
	page_table[page_table_index] = phys_addr | PAGE_FLAG_PRESENT | flags;
	num_virt_pages++;
	invalidate_tlb_entry(virt_addr);

	if (prev_page_dir != 0){
		sync_page_dirs();
		if (prev_page_dir != kernel_page_dir){
			change_page_dir_addr(prev_page_dir);
		}
	}
	return;
}

/*
* Gets the virtual address of the page directory:
*	- "cr3" has the physical address of the page directory
*	- adding KERNEL_START translates to virtual address
*/
uint32_t* get_page_dir_addr(){
	uint32_t phys_page_dir;
	asm volatile("MOV %%cr3, %0": "=r"(phys_page_dir));
	return (uint32_t*)(phys_page_dir + KERNEL_START);
}

/*
* Changes the address of the phyiscal page directory:
*	- takes in address of new virtual page_dir
*	- translates it to physical addr space
*	- sets it to "cr3"
*/
void change_page_dir_addr(uint32_t* virt_page_dir){
	uint32_t* phys_page_dir = (uint32_t*)(((uint32_t)virt_page_dir) - KERNEL_START);
	asm volatile("MOV %0, %%eax \n MOV %%eax, %%cr3 \n" :: "m"(phys_page_dir));
	return;
}

/*
* Synchronizes all entries of the main kernel page dir with all existing user page diretories
*	TODO: needs to be dynamic
*/
void sync_page_dirs(){
	for(int i = 0; i < NUM_PAGE_DIRS; i++){
		if(page_dirs_used[i]){
			uint32_t* page_dir = page_dirs[i];

			for(int j = 768; j < 1023; j++){
				page_dir[j] = kernel_page_dir[j] & ~PAGE_FLAG_OWNER;
			}
		}
	}
	return;
}
