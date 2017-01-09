#ifndef _H_INTERRUPTS_WRAPPER_
#define _H_INTERRUPTS_WRAPPER_

/** @file interrupts/wrapper.h
 *
 * This file contains headers for the short assembly functions that handle interrupts. These simply do the following:
 * * Push all the registers to the stack using `PUSHAD`.
 * * Call the respective C function.
 * * Pop all the registers from the stack using `POPAD`.
 * * If necessary, remove the error code from the stack.
 * * Does an `IRET` back to the code that was interrupted.
 *
 * Since all of these functions are just wrappers around functions defined elsewhere, and all behave in the same way,
 *  they will not be documented.
 *
 * @sa interrupts/exceptions.h
 */

void int_wrap_div0();
void int_wrap_nmi();
void int_wrap_overflow();
void int_wrap_bound_range_exceeded();
void int_wrap_invalid_opcode();
void int_wrap_double_fault();
void int_wrap_invalid_tss();
void int_wrap_segment_not_present();
void int_wrap_stack_segment_not_present();
void int_wrap_gpf();
void int_wrap_page_fault();
void int_wrap_floating_point();

#endif
