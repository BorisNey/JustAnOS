.section .text

.global gdt_flush_asm
.type gdt_flush_asm, @function
gdt_flush_asm:
    mov 4(%esp), %eax
    lgdt (%eax)
    mov $0x10, %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ljmp $0x08, $.flush
.flush:
    ret

.global tss_flush_asm
.type tss_flus_asm, @function
tss_flush_asm:
	mov $0x2B, %ax
	ltr %ax
	ret

