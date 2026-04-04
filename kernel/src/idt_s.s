.section .text

.global idtFlush
.type idtFlush, @function
idtFlush:
    movl 4(%esp), %eax
    lidt (%eax)
    sti
    ret

.macro ISR_NOERRORCODE num
.global isr\num
.type isr\num, @function
isr\num:
    cli
    pushl $0
    pushl $\num
    jmp isrCommonStub
.endm

.macro ISR_ERRORCODE num
.global isr\num
.type isr\num, @function
isr\num:
    cli
    pushl $\num
    jmp isrCommonStub
.endm

.macro IRQ num offset
.global irq\num
.type irq\num, @function
irq\num:
	cli
	pushl $0
	pushl $\offset
	jmp irqCommonStub
.endm

ISR_NOERRORCODE 0
ISR_NOERRORCODE 1
ISR_NOERRORCODE 2
ISR_NOERRORCODE 3
ISR_NOERRORCODE 4
ISR_NOERRORCODE 5
ISR_NOERRORCODE 6
ISR_NOERRORCODE 7
ISR_ERRORCODE   8
ISR_NOERRORCODE 9
ISR_ERRORCODE   10
ISR_ERRORCODE   11
ISR_ERRORCODE   12
ISR_ERRORCODE   13
ISR_ERRORCODE   14
ISR_NOERRORCODE 15
ISR_NOERRORCODE 16
ISR_ERRORCODE 17
ISR_NOERRORCODE 18
ISR_NOERRORCODE 19
ISR_NOERRORCODE 20
ISR_NOERRORCODE 21
ISR_NOERRORCODE 22
ISR_NOERRORCODE 23
ISR_NOERRORCODE 24
ISR_NOERRORCODE 25
ISR_NOERRORCODE 26
ISR_NOERRORCODE 27
ISR_NOERRORCODE 28
ISR_NOERRORCODE 29
ISR_NOERRORCODE 30
ISR_NOERRORCODE 31

IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

ISR_NOERRORCODE 128
ISR_NOERRORCODE 177

.extern isrHandler
.global isrCommonStub
.type isrCommonStub, @function
isrCommonStub:
    pushal
    movl %ds, %eax
    pushl %eax
    movl %cr2, %eax
    pushl %eax

    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    pushl %esp
    call isrHandler

    addl $8, %esp
    popl %ebx
    movw %bx, %ds
    movw %bx, %es
    movw %bx, %fs
    movw %bx, %gs

    popal
    addl $8, %esp
    sti
    iret

.extern irqHandler
.global irqCommonStub
.type irqCommonStub, @function
irqCommonStub:
    pushal
    movl %ds, %eax
    pushl %eax
    movl %cr2, %eax
    pushl %eax

    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    pushl %esp
    call irqHandler

    addl $8, %esp
    popl %ebx
    movw %bx, %ds
    movw %bx, %es
    movw %bx, %fs
    movw %bx, %gs

    popal
    addl $8, %esp
    sti
    iret

