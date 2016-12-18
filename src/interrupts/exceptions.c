#include <stdint.h>

#include "interrupts/exceptions.h"
#include "interrupts/wrapper.h"
#include "interrupts/idt.h"
#include "main/panic.h"
#include "mem/gdt.h"

void except_div0(idt_proc_state_t state) {
    (void)state;
    panic("Division by 0");
}

void except_nmi(idt_proc_state_t state) {
    (void)state;
    panic("NMI Received (Hardware Failure?)");
}

void except_overflow(idt_proc_state_t state) {
    (void)state;
    panic("Overflow Exception");
}

void except_bound_range_exceeded(idt_proc_state_t state) {
    (void)state;
    panic("Bound Range Exceeded Exception");
}

void except_invalid_opcode(idt_proc_state_t state) {
    (void)state;
    panic("Invalid Opcode");
}

void except_double_fault(idt_proc_state_t state, uint32_t errcode) {
    (void)state;
    panic("Double Fault %x", errcode);
}

void except_invalid_tss(idt_proc_state_t state, uint32_t errcode) {
    (void)state;
    panic("Invalid TSS %x", errcode);
}

void except_segment_not_present(idt_proc_state_t state, uint32_t errcode) {
    (void)state;
    panic("Segment not Present %x", errcode);
}

void except_stack_segment_not_present(idt_proc_state_t state, uint32_t errcode) {
    (void)state;
    (void)errcode;
    panic("Stack-Segment not Present %x", errcode);
}

void except_gpf(idt_proc_state_t state, uint32_t errcode) {
    (void)state;
    panic("General Protection Fault %x", errcode);
}

void except_page_fault(idt_proc_state_t state, uint32_t errcode) {
    (void)state;
    panic("Page Fault %x", errcode);
}

void except_floating_point(idt_proc_state_t state) {
    (void)state;
    panic("Floating Point Exception");
}

void except_init() {
    idt_install(EXCEPT_DIV0, (uint32_t)int_wrap_div0, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install(EXCEPT_NMI, (uint32_t)int_wrap_nmi, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install(EXCEPT_OVERFLOW, (uint32_t)int_wrap_overflow, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install(EXCEPT_BOUND_RANGE_EXCEEDED,
        (uint32_t)int_wrap_bound_range_exceeded, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install(EXCEPT_INVALID_OPCODE, (uint32_t)int_wrap_invalid_opcode, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install(EXCEPT_DOUBLE_FAULT, (uint32_t)int_wrap_double_fault, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install(EXCEPT_INVALID_TSS, (uint32_t)int_wrap_invalid_tss, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install(EXCEPT_SEGMENT_NOT_PRESENT,
        (uint32_t)int_wrap_segment_not_present, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install(EXCEPT_STACK_SEGMENT_NOT_PRESENT,
        (uint32_t)int_wrap_stack_segment_not_present, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install(EXCEPT_GPF, (uint32_t)int_wrap_gpf, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install(EXCEPT_PAGE_FAULT, (uint32_t)int_wrap_page_fault, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install(EXCEPT_FLOATING_POINT, (uint32_t)int_wrap_floating_point, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
}
