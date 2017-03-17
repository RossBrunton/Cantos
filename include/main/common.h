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

#define TOTAL_VM_SIZE 0x100000000
#define KERNEL_VM_SIZE 0x40000000
#define KERNEL_VM_PAGES (KERNEL_VM_SIZE / PAGE_SIZE)
#define KERNEL_VM_PAGE_TABLES (KERNEL_VM_PAGES / PAGE_TABLE_LENGTH)
#define KERNEL_VM_BASE (TOTAL_VM_SIZE - KERNEL_VM_SIZE) // Higher half kernel

#define MAX_CORES 32

#endif
