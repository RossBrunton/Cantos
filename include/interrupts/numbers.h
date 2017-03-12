#ifndef _H_INTERRUPTS_NUMBERS_
#define _H_INTERRUPTS_NUMBERS_

/** @file interrupts/numbers.h
 *
 * Constants representing all the exceptions that can be raised.
 * 
 * @sa http://wiki.osdev.org/Exceptions
 */

/*
 *
 *
 * CPU Exceptions
 *
 * 
 */

/** (\#DE) Division by zero exception */
#define INT_DIV0 0x0
/** (\#DB) Debug exception
 *
 * This is not handled.
 */
#define INT_DEBUG 0x1
/** Non maskable interrupt
 *
 * This typically means a hardware failure.
 */
#define INT_NMI 0x2
/** (\#BP) Breakpoint exception
 *
 * This is not handled.
 */
#define INT_BREAKPOINT 0x3
/** (\#OF) Overflow exception */
#define INT_OVERFLOW 0x4
/** (\#BR) Bound range exceeded exception */
#define INT_BOUND_RANGE_EXCEEDED 0x5
/** (\#UD) Invalid opcode exception */
#define INT_INVALID_OPCODE 0x6
/** (\#NM) In theory this is raised when a floating point operation is requested and there is no FPU.
 *
 * Most CPUs have a built in FPU, so this probably won't be raised, and so is not handled.
 */
#define INT_DEVICE_NOT_AVAILABLE 0x7
/** (\#DF) Raised when there is a page fault handling a page fault */
#define INT_DOUBLE_FAULT 0x8
/** (\#TS) Invalid segment selector */
#define INT_INVALID_TSS 0xa
/** (\#NP) Raised when a segment is not present */
#define INT_SEGMENT_NOT_PRESENT 0xb
/** (\#SS) Raised when a stack segment is not present */
#define INT_STACK_SEGMENT_NOT_PRESENT 0xc
/** (\#GP) Raised on a general protection fault.
 *
 * This generally means a process has tried to do something it shouldn't, such as running a priviliged instruction.
 */
#define INT_GPF 0xd
/** (\#PF) Raised on a page fault.
 *
 * That is, a memory access for memory in a page which isn't present.
 */
#define INT_PAGE_FAULT 0xe
/** (\#MF) Raised on a floating point exception */
#define INT_FLOATING_POINT 0x10
/** (\#AC) Exception raised when alignment checking is enabled, and an out of alignment memory access is performed
 *
 * Alignment checking is not enabled, and so this will never get raised and there is no handler.
 */
#define INT_ALIGNMENT_CHECK 0x11
/** (\#MC) Machine check exceptions, raised by the hardware for hardware specific errors
 *
 * This is never enabled, will never be raised and has no handler.
 */
#define INT_MACHINE_CHECK 0x12
/** (\#XM) Raised on an 128 bit floating point exception is raised.
 *
 * This is never enabled, will never be raised and has no handler.
 */
#define INT_SIMD_FLOATING_POINT 0x13
/** (\#VE) Raised on a virtualization exception
 *
 * This is never set up, will never be raised and has no handler.
 */
#define INT_VIRTUALIZATION 0x14
/** (\#SX) Security exception in host
 *
 * Not sure when this is raised, but there is no handler.
 */
#define INT_SECURITY 0x1e

/*
 *
 *
 * LAPIC Exceptions
 *
 * 
 */

/** The base of the LAPIC exceptions
 *
 * All LAPIC exceptions are mapped from INT_LAPIC_BASE to @ref INT_IOAPIC_BASE.
 */
#define INT_LAPIC_BASE 0x20


/** Offset of the LAPIC timer interrupt
 */
#define INT_LAPIC_TIMER 0x0

/** Offset of the LAPIC thermal interrupt
 */
#define INT_LAPIC_THERMAL 0x1

/** Offset of the LAPIC performance interrupt
 */
#define INT_LAPIC_PERFORMANCE 0x2

/** Offset of the first LAPIC local interrupt
 */
#define INT_LAPIC_LINT1 0x3

/** Offset of the second LAPIC local interrupt
 */
#define INT_LAPIC_LINT2 0x4

/** Offset of the LAPIC error interrupt
 */
#define INT_LAPIC_ERROR 0x5

/*
 *
 *
 * IOAPIC Exceptions
 *
 * 
 */

/** The base of the IOAPIC exceptions
 *
 * The interrupt vector of IRQ #x is (INT_IOAPIC_BASE + x).
 */
#define INT_IOAPIC_BASE 0x30

/*
 *
 *
 * IRQ Interrupts
 *
 * 
 */

#define INT_IRQ_INTERRUPT_TIMER 0x0
#define INT_IRQ_KEYBOARD 0x1
#define INT_IRQ_CASCADE 0x2
#define INT_IRQ_INTERRUPT_TIMER_IOAPIC 0x2
#define INT_IRQ_COM2 0x3
#define INT_IRQ_COM1 0x4
#define INT_IRQ_LPT2 0x5
#define INT_IRQ_FLOPPY 0x6
#define INT_IRQ_LPT1 0x7 /* Also "unreliable "spurious" interrupt" */
#define INT_IRQ_CMOS 0x8
#define INT_IRQ_9 0x9
#define INT_IRQ_10 0xa
#define INT_IRQ_11 0xb
#define INT_IRQ_MOUSE 0xc
#define INT_IRQ_FPU 0xd
#define INT_IRQ_PATA 0xe
#define INT_IRQ_SATA 0xf

#endif
