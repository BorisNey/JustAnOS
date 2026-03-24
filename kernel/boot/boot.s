/*
	Declare constants for the multiboot header.
*/
.set ALIGN,    1<<0                       /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1                       /* provide memory map */
.set USE_GFX,  0                          /* for video memory access */
.set FLAGS,    ALIGN | MEMINFO | USE_GFX  /* this is the Multiboot 'flag' field */ 
.set MAGIC,    0x1BADB002                 /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS)           /* checksum of above, to prove we are multiboot */


/* 
	Multiboot header with multiboot standard
*/
.section .multiboot, "a"
.balign 4
	.long MAGIC
	.long FLAGS
	.long CHECKSUM
	.long 0, 0, 0, 0, 0
	.long 0
	.long 800
	.long 600
	.long 32


/*
	Stack pointer register (esp) with 16384 bytes
	On x86 must be 16-byte aligned (System V ABI standard)
*/
.section .bss
.align 16
stack_bottom:
	.skip 16384 * 8 # 16 KiB * 8 = 128KiB, more than enough initially
stack_top:


/*
	Boot section - runs before paging
*/
.section .boot, "ax"
.global _start
.type _start, @function
_start:
	/*
		"intitila_page_dir" is virtual, substraction converts to physical
		cr3 is the Page Directory Base Register
	*/
	movl $(kernel_page_dir - 0xC0000000), %ecx
	movl %ecx, %cr3
	
	/*
		Bit 4 of cr4 enables Page Size Extension
		Setting Page Size to 4MB
	*/
	mov %cr4, %ecx
	orl $0x10, %ecx
	mov %ecx, %cr4
	
	/*
		Bit 31 of cr0 enables paging
		after this moment all memory access goes through cr3 (pointing to intitial_page_dir)
	*/
	movl %cr0, %ecx
	orl $0x80000000, %ecx
	movl %ecx, %cr0
	
	/*
		Jumping to 0xC0000000
	*/
	jmp higher_half


/*
	Higher half - runs after paging enabled
*/
.section .text
.extern kernel_main
.global higher_half
.type higher_half, @function
higher_half:
    movl $stack_top, %esp    # set up the stack
	addl $0xC0000000, %ebx   # convert to virtual address
    pushl %ebx               # push Multiboot info pointer (physical address)
    xorl %ebp, %ebp          # clear base pointer (marks end of stack frames)
    call kernel_main         # call your C kernel
halt:
    hlt                      # halt the CPU
    jmp halt                 # if an interrupt wakes it, halt again


/*
	Page directory
*/
.section .data
.global kernel_page_dir
.balign 4096
kernel_page_dir:
    .long 0b10000011                   # entry 0: maps virtual 0x00000000 → physical 0x00000000 (4MB)
    .fill 767, 4, 0                     # entries 1–767: unmapped
    .long (0 << 22) | 0b10000011       # entry 768 (0xC0000000): maps → physical 0x00000000
    .long (1 << 22) | 0b10000011       # entry 769 (0xC0400000): maps → physical 0x00400000
    .long (2 << 22) | 0b10000011       # entry 770 (0xC0800000): maps → physical 0x00800000
    .long (3 << 22) | 0b10000011       # entry 771 (0xC0C00000): maps → physical 0x00C00000
    .fill 252, 4, 0                     # entries 772–1023: unmapped


/*
This is a **4KB-aligned page directory** with 1024 entries, each covering **4MB** (since PSE is enabled).

The flags `0b10000011` break down as:
| Bit | Flag       | Meaning           |
|-----|------------|-------------------|
| 0   | Present    | Page is in memory |
| 1   | Read/Write | Writable          |
| 7   | Page Size  | 4MB page (requires PSE) |

**Entry 0** maps virtual `0x00000000 → 0x00000000`. This is a **temporary identity map** so the CPU can keep executing at physical addresses right after paging is enabled (before the `jmp higher_half`).

**Entries 768–771** map virtual `0xC0000000–0xC0FFFFFF → physical 0x00000000–0x00FFFFFF`. This is the **higher-half kernel mapping** — 16MB of physical RAM made accessible at `0xC0000000`. Your kernel is linked to run here.

The identity map at entry 0 is typically **removed later** (zeroed out) once the kernel is running safely in the higher half, to catch null pointer dereferences.


Physical RAM        Virtual Address Space
0x00000000  ──────► 0x00000000   (identity map, temporary)
                    ...
0x00000000  ──────► 0xC0000000   (higher half kernel)
0x00400000  ──────► 0xC0400000
0x00800000  ──────► 0xC0800000
0x00C00000  ──────► 0xC0C00000
*/
