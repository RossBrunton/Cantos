#ifndef _H_INTERRUPTS_EXCEPTIONS_
#define _H_INTERRUPTS_EXCEPTIONS_

/** @file interrupts/exceptions.h
 *
 * Constants representing CPU exceptions that can occur, and handler functions which will be called when they do.
 *
 * There is no reason to call any of these functions, as they will automatically be called from the IDT. The
 *  documentation here will simply explain what happens when such an exception is raised.
 * 
 * Since the IDT cannot call C functions directly, some wrapper functions exist to bridge the gap, which live in
 *  @ref wrapper.h.
 * 
 * @sa http://wiki.osdev.org/Exceptions
 */

#include "interrupts/idt.h"
#include "main/common.h"

/** (\#DE) Division by zero exception */
#define EXCEPT_DIV0 0x0
/** (\#DB) Debug exception
 *
 * This is not handled.
 */
#define EXCEPT_DEBUG 0x1
/** Non maskable interrupt
 *
 * This typically means a hardware failure.
 */
#define EXCEPT_NMI 0x2
/** (\#BP) Breakpoint exception
 *
 * This is not handled.
 */
#define EXCEPT_BREAKPOINT 0x3
/** (\#OF) Overflow exception */
#define EXCEPT_OVERFLOW 0x4
/** (\#BR) Bound range exceeded exception */
#define EXCEPT_BOUND_RANGE_EXCEEDED 0x5
/** (\#UD) Invalid opcode exception */
#define EXCEPT_INVALID_OPCODE 0x6
/** (\#NM) In theory this is raised when a floating point operation is requested and there is no FPU.
 *
 * Most CPUs have a built in FPU, so this probably won't be raised, and so is not handled.
 */
#define EXCEPT_DEVICE_NOT_AVAILABLE 0x7
/** (\#DF) Raised when there is a page fault handling a page fault */
#define EXCEPT_DOUBLE_FAULT 0x8
/** (\#TS) Invalid segment selector */
#define EXCEPT_INVALID_TSS 0xa
/** (\#NP) Raised when a segment is not present */
#define EXCEPT_SEGMENT_NOT_PRESENT 0xb
/** (\#SS) Raised when a stack segment is not present */
#define EXCEPT_STACK_SEGMENT_NOT_PRESENT 0xc
/** (\#GP) Raised on a general protection fault.
 *
 * This generally means a process has tried to do something it shouldn't, such as running a priviliged instruction.
 */
#define EXCEPT_GPF 0xd
/** (\#PF) Raised on a page fault.
 *
 * That is, a memory access for memory in a page which isn't present.
 */
#define EXCEPT_PAGE_FAULT 0xe
/** (\#MF) Raised on a floating point exception */
#define EXCEPT_FLOATING_POINT 0x10
/** (\#AC) Exception raised when alignment checking is enabled, and an out of alignment memory access is performed
 *
 * Alignment checking is not enabled, and so this will never get raised and there is no handler.
 */
#define EXCEPT_ALIGNMENT_CHECK 0x11
/** (\#MC) Machine check exceptions, raised by the hardware for hardware specific errors
 *
 * This is never enabled, will never be raised and has no handler.
 */
#define EXCEPT_MACHINE_CHECK 0x12
/** (\#XM) Raised on an 128 bit floating point exception is raised.
 *
 * This is never enabled, will never be raised and has no handler.
 */
#define EXCEPT_SIMD_FLOATING_POINT 0x13
/** (\#VE) Raised on a virtualization exception
 *
 * This is never set up, will never be raised and has no handler.
 */
#define EXCEPT_VIRTUALIZATION 0x14
/** (\#SX) Security exception in host
 *
 * Not sure when this is raised, but there is no handler.
 */
#define EXCEPT_SECURITY 0x1e

/** The base of the LAPIC exceptions
 *
 * All LAPIC exceptions are mapped from EXCEPT_LAPIC_BASE to @ref EXCEPT_IOAPIC_BASE.
 */
#define EXCEPT_LAPIC_BASE 0x20

/** The base of the IOAPIC exceptions
 *
 * The interrupt vector of IRQ #x is (EXCEPT_IOAPIC_BASE + x).
 */
#define EXCEPT_IOAPIC_BASE 0x30

/** Handles \#DE by panicing
 *
 * @param[in] state The values of all the registers
 */
void except_div0(idt_proc_state_t state);
/** Handles an NMI by panicing
 *
 * @param[in] state The values of all the registers
 */
void except_nmi(idt_proc_state_t state);
/** Handles \#OF by panicing
 *
 * @param[in] state The values of all the registers
 */
void except_overflow(idt_proc_state_t state);
/** Handles \#BR by panicing
 *
 * @param[in] state The values of all the registers
 */
void except_bound_range_exceeded(idt_proc_state_t state);
/** Handles \#UD by panicing
 *
 * @param[in] state The values of all the registers
 */
void except_invalid_opcode(idt_proc_state_t state);
/** Handles \#DF by panicing
 *
 * @param[in] state The values of all the registers
 * @param[in] errcode Always 0
 */
void except_double_fault(idt_proc_state_t state, uint32_t errcode);
/** Handles \#TS by panicing
 *
 * @param[in] state The values of all the registers
 * @param[in] errcode The selector index of the invalid TSS
 */
void except_invalid_tss(idt_proc_state_t state, uint32_t errcode);
/** Handles \#NP by panicing
 *
 * @param[in] state The values of all the registers
 * @param[in] errcode The selector index of the invalid segment
 */
void except_segment_not_present(idt_proc_state_t state, uint32_t errcode);
/** Handles \#SS by panicing
 *
 * @param[in] state The values of all the registers
 * @param[in] errcode The selector index of the not present segment, or 0
 */
void except_stack_segment_not_present(idt_proc_state_t state, uint32_t errcode);
/** Handles \#GP by panicing
 *
 * @param[in] state The values of all the registers
 * @param[in] errcode The selector index of the segment selector when it is at fault, otherwise 0
 */
void except_gpf(idt_proc_state_t state, uint32_t errcode);
/** Handles \#PF by panicing, which displays the invalid address
 *
 * @param[in] state The values of all the registers
 * @param[in] errcode Information about the page fault
 */
void except_page_fault(idt_proc_state_t state, uint32_t errcode);
/** Handles \#MF by panicing
 *
 * @param[in] state The values of all the registers
 */
void except_floating_point(idt_proc_state_t state);

/** Registers the handlers for all the exceptions into the IDT.
 *
 * @ref idt_init must be called before this to set up the IDT.
 */
void except_init();

#endif
