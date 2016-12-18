#ifndef _H_INTERRUPTS_WRAPPER_
#define _H_INTERRUPTS_WRAPPER_

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
