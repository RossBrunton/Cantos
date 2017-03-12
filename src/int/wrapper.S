.section .text

#include "int/numbers.h"

.macro handle vec, name
.globl idt_asm_interrupt_\name
idt_asm_interrupt_\name:
    pushal
    push $\vec
    cld
    call idt_handle
    add $4, %esp # Remove the vector
    popal
    iret
.endm

.macro handlee vec, name
.globl idt_asm_interrupt_\name
idt_asm_interrupt_\name:
    pushal
    push $\vec
    cld
    call idt_handle_with_error
    add $4, %esp # Remove the vector
    popal
    add $4, %esp # Remove the error code
    iret
.endm

# Processor Exceptions
handle INT_DIV0 div0
handle INT_NMI nmi
handle INT_OVERFLOW overflow
handle INT_BOUND_RANGE_EXCEEDED bre
handle INT_INVALID_OPCODE invalidop
handle INT_FLOATING_POINT floatingpoint

handlee INT_DOUBLE_FAULT doublefault
handlee INT_INVALID_TSS invalidtss
handlee INT_SEGMENT_NOT_PRESENT snp
handlee INT_STACK_SEGMENT_NOT_PRESENT ssnp
handlee INT_GPF gpf
handlee INT_PAGE_FAULT pagefault

# IOapic
handle INT_IOAPIC_BASE + INT_IRQ_KEYBOARD iokeyboard
handle INT_IOAPIC_BASE + INT_IRQ_INTERRUPT_TIMER_IOAPIC iotimer
handle INT_IOAPIC_BASE + INT_IRQ_COM2 iocom1
handle INT_IOAPIC_BASE + INT_IRQ_COM1 iocom2
handle INT_IOAPIC_BASE + INT_IRQ_LPT2 iolpt2
handle INT_IOAPIC_BASE + INT_IRQ_FLOPPY iofloppy
handle INT_IOAPIC_BASE + INT_IRQ_LPT1 iolpt1
handle INT_IOAPIC_BASE + INT_IRQ_CMOS iocmos
handle INT_IOAPIC_BASE + INT_IRQ_9 io9
handle INT_IOAPIC_BASE + INT_IRQ_10 io10
handle INT_IOAPIC_BASE + INT_IRQ_11 io11
handle INT_IOAPIC_BASE + INT_IRQ_MOUSE iomouse
handle INT_IOAPIC_BASE + INT_IRQ_FPU iofpu
handle INT_IOAPIC_BASE + INT_IRQ_PATA iopata
handle INT_IOAPIC_BASE + INT_IRQ_SATA iosata

# Lapic
handle INT_LAPIC_BASE + INT_LAPIC_TIMER ltimer
