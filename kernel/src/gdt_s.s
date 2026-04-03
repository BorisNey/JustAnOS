.section .text

.global gdtFlush
.type gdtFlush, @function
gdtFlush:
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

.global tssFlush
.type tssFlush, @function
tssFlush:
	mov $0x2B, %ax
	ltr %ax
	ret

