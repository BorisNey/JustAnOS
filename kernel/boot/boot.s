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
	.skip 16384 * 8 # 16 KiB * 8
stack_top:


/*
	Boot section - runs before paging
*/
.section .boot, "ax"
.global _start
.type _start, @function
_start:
	movl $(initial_page_dir - 0xC0000000), %ecx
	movl %ecx, %cr3
	
	mov %cr4, %ecx
	orl $0x10, %ecx
	mov %ecx, %cr4
	
	movl %cr0, %ecx
	orl $0x80000000, %ecx
	movl %ecx, %cr0
	
	jmp higher_half


/*
	Higher half - runs after paging
*/
.section .text
.extern kernel_main
.global higher_half
.type higher_half, @function
higher_half:
    movl $stack_top, %esp
    pushl %ebx
    pushl %eax
    xorl %ebp, %ebp
    call kernel_main

halt:
    hlt
    jmp halt


/*
	Page directory
*/
.section .data
.balign 4096
.global initial_page_dir
.type initial_page_dir, @function
initial_page_dir:
	.long 0b10000011
	.fill 767, 4, 0
	.long (0 << 22) | 0b10000011
	.long (1 << 22) | 0b10000011
	.long (2 << 22) | 0b10000011
	.long (3 << 22) | 0b10000011
	.fill 252, 4, 0

