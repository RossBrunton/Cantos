#ifndef _H_INTERRUPTS_EXCEPTIONS_
#define _H_INTERRUPTS_EXCEPTIONS_

#include "interrupts/idt.h"

#define EXCEPT_DIV0 0x0
#define EXCEPT_DEBUG 0x1
#define EXCEPT_NMI 0x2
#define EXCEPT_BREAKPOINT 0x3
#define EXCEPT_OVERFLOW 0x4
#define EXCEPT_BOUND_RANGE_EXCEEDED 0x5
#define EXCEPT_INVALID_OPCODE 0x6
#define EXCEPT_DEVICE_NOT_AVAILABLE 0x7 /* This is only for the FPU */
#define EXCEPT_DOUBLE_FAULT 0x8
#define EXCEPT_INVALID_TSS 0xa
#define EXCEPT_SEGMENT_NOT_PRESENT 0xb
#define EXCEPT_STACK_SEGMENT_NOT_PRESENT 0xc
#define EXCEPT_GPF 0xd
#define EXCEPT_PAGE_FAULT 0xe
#define EXCEPT_FLOATING_POINT 0x10
#define EXCEPT_ALIGNMENT_CHECK 0x11
#define EXCEPT_MACHINE_CHECK 0x12
#define EXCEPT_SIMD_FLOATING_POINT 0x13
#define EXCEPT_VIRTUALIZATION 0x14
#define EXCEPT_SECURITY 0x1e

void except_div0(idt_proc_state_t state);
void except_nmi(idt_proc_state_t state);
void except_overflow(idt_proc_state_t state);
void except_bound_range_exceeded(idt_proc_state_t state);
void except_invalid_opcode(idt_proc_state_t state);
void except_double_fault(idt_proc_state_t state, uint32_t errcode);
void except_invalid_tss(idt_proc_state_t state, uint32_t errcode);
void except_segment_not_present(idt_proc_state_t state, uint32_t errcode);
void except_stack_segment_not_present(idt_proc_state_t state, uint32_t errcode);
void except_gpf(idt_proc_state_t state, uint32_t errcode);
void except_page_fault(idt_proc_state_t state, uint32_t errcode);
void except_floating_point(idt_proc_state_t state);

void except_init();

#endif
