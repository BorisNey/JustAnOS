#include "memory.h"

/*
IMPROVEMENTS:
	- page fault handling (idt 11)
	- guard pages for potential stack overflow
*/

extern uint32_t g_kernel_end; // End of kernelcode in linker.ld
extern uint32_t g_kernel_page_dir[1024]; // gets initialized in boot.s

static uint32_t g_page_frame_min; // the first frame number that is safe to allocate (everything below is kernel/modules)
static uint32_t g_page_frame_max; // the last frame number based on how much RAM the machine has.
static uint8_t g_phys_mem_bitmap[NUM_PAGE_FRAMES / 8];  // each bit tracks one 4KB physical page frame (1 allocated, 0 free)
static kll_node* g_proc_pd_kll = NULL; // Dynamic list of Headers for process page directories

void initMemory(mb_info_t* boot_info){
	/*
	* Calculates the next free page after kernel, which is safe to allocate to
	* 	+ 0xFFF: the next phys page
	*	& ~0xFFF: masks off the lower 12 bits -> start of the next 4KB phys page
	*/
	uint32_t phys_alloc_start = ((uint32_t)&g_kernel_end - KERNEL_START + 0xFFF) & ~0xFFF;

	/*
	* Calculates last accessible address
	*	* 1024, because boot_info->mem_upper is in KB
	*/
	uint32_t phys_mem_high = boot_info->mem_upper * 1024;

	/*
	* Clears the entry
	*	- marking that 4MB region as not present for null pointer dereferencing
	*	- making an access to virt 0x0 results in a page fault
	*/
	g_kernel_page_dir[0] = 0;
	invalidateTLBEntry(0);

	/*
	* Setting up recursive virtual address to the page_dir
	*	- invalidating updates the tlb entry
	*/
	g_kernel_page_dir[1023] = ((uint32_t) g_kernel_page_dir - KERNEL_START) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;
	invalidateTLBEntry(REC_PAGE_DIR);
	
	initPMM(phys_alloc_start, phys_mem_high);

	biosTermPrintf("DBG: Memory init success\n");
	return;
}

/*
* Flushes the Translation Lookaside Buffer (in CPU) for that address
* 	e.g. for updating the address
*/
void invalidateTLBEntry(uint32_t virt_addr){
	asm volatile("invlpg (%0)" :: "r"(virt_addr));
	return;
}

/*
* Initialization of the Physical Memory Management
*/
void initPMM(uint32_t mem_low, uint32_t mem_high){
	g_page_frame_min = CEIL_DIV(mem_low, PAGE_SIZE);
	g_page_frame_max = mem_high / PAGE_SIZE;

	memset(g_phys_mem_bitmap, 0, sizeof(g_phys_mem_bitmap));
	return;
}


/*
* Finds a free pyhsical page frame, sets it as allocated and returns its physical address
*/
uint32_t allocPageFrame(){
	uint32_t start = g_page_frame_min / 8 + ((g_page_frame_min & 7) != 0 ? 1 : 0);
	uint32_t end = g_page_frame_max / 8 - ((g_page_frame_max & 7) != 0 ? 1 : 0);

	for (uint32_t b = start; b < end; b++){
		/*
		* Goes thru every pyhsical page frame and checks if allocated
		*/
		uint8_t byte = g_phys_mem_bitmap[b];
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
				g_phys_mem_bitmap[b] = byte;

				uint32_t addr = (b * 8 + i) * PAGE_SIZE;
				return addr;
			}
		}
	}

	// no free page frame found
	return 0;
}

/*
* Makes an entry in the kernel page directory, that links the physicall address to the virtual address with the specified flags
*/
void mapAddr(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags){
	// making sure that the current page directory is the right one
	uint32_t* prev_page_dir = 0;
	if(virt_addr >= KERNEL_START){
		prev_page_dir = getCurrPageDirReg();
		if(prev_page_dir != g_kernel_page_dir){
			setCurrPageDirReg(g_kernel_page_dir);
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

	uint32_t* page_dir = (uint32_t*)REC_PAGE_DIR;
	uint32_t* page_table = (uint32_t*)REC_PAGE_TABLE(page_dir_index);

	/*
	* If the page_dir entry of the virtual address is not present, 
	* we have to allocate in physical space (with pmm) and
	* make an fresh (all 0) entry in our page directory
	*/
	if(!(page_dir[page_dir_index] & PAGE_FLAG_PRESENT)){
		uint32_t page_table_addr = allocPageFrame();
		page_dir[page_dir_index] = page_table_addr | PAGE_FLAG_PRESENT |PAGE_FLAG_WRITE | PAGE_FLAG_OWNER | flags;
		invalidateTLBEntry(virt_addr);
		
		for(uint32_t i = 0; i < 1024; i++){
			page_table[i] = 0;
		}
	}


	// Actually mapping the virtual to the physical address
	page_table[page_table_index] = phys_addr | PAGE_FLAG_PRESENT | flags;
	invalidateTLBEntry(virt_addr);

	// if the changes above were made in the kernel space, then all page dirs have to be synced again
	if (prev_page_dir != 0){
		syncPageDirs();
		if (prev_page_dir != g_kernel_page_dir){
			setCurrPageDirReg(prev_page_dir);
		}
	}
	return;
}

/*
* Gets the virtual address of the page directory:
*	- "cr3" has the physical address of the page directory
*	- adding KERNEL_START translates to virtual address
*/
uint32_t* getCurrPageDirReg(){
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
void setCurrPageDirReg(uint32_t* virt_page_dir){
	uint32_t* phys_page_dir = (uint32_t*)(((uint32_t)virt_page_dir) - KERNEL_START);
	asm volatile("MOV %0, %%eax \n MOV %%eax, %%cr3 \n" :: "m"(phys_page_dir));
	return;
}


/*
* Has to be called when creating a new process
* Creates a page directory for that process and syncs it with all the other existing page dirs
* Returns the pyhsicall address of the page dir, that has to be loaded into cr3
*/
proc_pd_header_t* createProcPageDir(unsigned int id){
	proc_pd_header_t* proc_pd_header = (proc_pd_header_t*)kmalloc(sizeof(proc_pd_header_t));
	if (proc_pd_header == NULL){
		biosTermPrintf("ERR: Kmalloc\n");
		return 0;
	}
	g_proc_pd_kll = kllAddNode(g_proc_pd_kll, proc_pd_header);

	proc_pd_header->id = id;

	// allocate a pageframe of the page directory
	uint32_t new_page_dir_phys = allocPageFrame();
	proc_pd_header->page_dir_phys = new_page_dir_phys;

	// Temporary virt mapping of the page dir
	uint32_t* new_page_dir_virt = (uint32_t*)TEMP_MAP_ADDR;
	mapAddr((uint32_t)new_page_dir_virt, new_page_dir_phys, PAGE_FLAG_WRITE);

	// Copy kernel mappings
	memset((uint32_t*)new_page_dir_virt, 0, 4096);
    for (int i = 768; i < 1023; i++) {
        new_page_dir_virt[i] = g_kernel_page_dir[i] & ~PAGE_FLAG_OWNER;
    }

    // Recursive entry
    new_page_dir_virt[1023] = new_page_dir_phys | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;

    // Clear the temporary virt mapping
    uint32_t dir_index = TEMP_MAP_ADDR >> 22;
    uint32_t table_index = (TEMP_MAP_ADDR >> 12) & 0x3FF;
    uint32_t* page_table = (uint32_t*)REC_PAGE_TABLE(dir_index);
    page_table[table_index] = 0;
    invalidateTLBEntry(TEMP_MAP_ADDR);

	biosTermPrintf("DBG: New process page directory with ID: %d created\n", id);
	return proc_pd_header;
}


/*
* Synchronizes all entries of the kernel page directory with all existing process page directories
*/
void syncPageDirs(){
	unsigned int proc_pd_count = kllGetLength(g_proc_pd_kll);
	proc_pd_header_t* proc_pd_header;

	for (unsigned int i = 0; i < proc_pd_count; i++){
		proc_pd_header = kllGetData(g_proc_pd_kll, i);
		for(int j = 768; j < 1023; j++){
			/*
			BUG:
			page_dir_phys holds a physical address, but you're dereferencing it as a pointer — which the MMU interprets as a virtual address.
			With paging enabled, this writes to whatever the virtual address happens to map to (or page faults if unmapped).
			You'd need to temporarily map each process page directory before writing to it, similar to what you do in createProcPageDir.
			*/
			((uint32_t*)(proc_pd_header->page_dir_phys))[j] = g_kernel_page_dir[j] & ~PAGE_FLAG_OWNER;
		}
	}
	return;
}
