#ifndef _H_MAIN_COMMON_
#define _H_MAIN_COMMON_

/** @file main/common.h
 *
 * Contains common definitions used by multiple files and which aren't specific to any one.
 */

/** Processor state, in the same format as pushed by the `PUSHAD` instruction.
 *
 * That is, `PUSHAD` before a function call will have a structure of this type as its argument.
 */
typedef struct idt_proc_state_s {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
} idt_proc_state_t;

#define IRQ_INTERRUPT_TIMER 0x0
#define IRQ_KEYBOARD 0x1
#define IRQ_CASCADE 0x2
#define IRQ_INTERRUPT_TIMER_IOAPIC 0x2
#define IRQ_COM2 0x3
#define IRQ_COM1 0x4
#define IRQ_LPT2 0x5
#define IRQ_FLOPPY 0x6
#define IRQ_LPT1 0x7 /* Also "unreliable "spurious" interrupt" */
#define IRQ_CMOS 0x8
#define IRQ_9 0x9
#define IRQ_10 0xa
#define IRQ_11 0xb
#define IRQ_MOUSE 0xc
#define IRQ_FPU 0xd
#define IRQ_PATA 0xe
#define IRQ_SATA 0xf

/** Variables of this type represent a physical address
 *
 * That is, an address in physical memory, ignoring the page table.
 *
 * This can be cast to and from a void * freely.
 */
typedef uintptr_t addr_phys_t;

/** Variables of this type represent a logical address
 *
 * That is, an address after paging has been applied.
 *
 * This can be cast to and from a void * freely.
 */
typedef uintptr_t addr_logical_t;

#endif
