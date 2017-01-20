.section .text

.macro handle name, cfunc
.globl \name
\name:
    pushal
    cld
    call \cfunc
    popal
    iret
.endm

.macro handlee name, cfunc
.globl \name
\name:
    pushal
    cld
    call \cfunc
    popal
    add $4, %esp # Remove the error code
    iret
.endm

handle int_wrap_div0, except_div0
handle int_wrap_nmi, except_nmi
handle int_wrap_overflow, except_overflow
handle int_wrap_bound_range_exceeded, except_bound_range_exceeded
handle int_wrap_invalid_opcode, except_invalid_opcode
handlee int_wrap_double_fault, except_double_fault
handlee int_wrap_invalid_tss, except_invalid_tss
handlee int_wrap_segment_not_present, except_segment_not_present
handlee int_wrap_stack_segment_not_present, except_stack_segment_not_present
handlee int_wrap_gpf, except_gpf
handlee int_wrap_page_fault, except_page_fault
handle int_wrap_floating_point, except_floating_point

handle int_wrap_io_keyboard, ioapic_keyboard

handle int_wrap_lapic_timer, lapic_timer
