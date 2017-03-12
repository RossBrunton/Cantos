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
