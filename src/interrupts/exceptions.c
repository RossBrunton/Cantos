#include <stdint.h>

#include "interrupts/exceptions.h"
#include "interrupts/idt.h"
#include "main/panic.h"
#include "mem/gdt.h"
#include "interrupts/numbers.h"

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
    uint32_t addr;
    __asm__("mov %%cr2, %0" : "=r"(addr));
    panic("Page Fault %x [Address: %p]", errcode, addr);
}

void except_floating_point(idt_proc_state_t state) {
    (void)state;
    panic("Floating Point Exception");
}

void except_init() {
    IDT_ALLOW_INTERRUPT(INT_DIV0, div0);
    IDT_ALLOW_INTERRUPT(INT_NMI, nmi);
    IDT_ALLOW_INTERRUPT(INT_OVERFLOW, overflow);
    IDT_ALLOW_INTERRUPT(INT_BOUND_RANGE_EXCEEDED, bre);
    IDT_ALLOW_INTERRUPT(INT_INVALID_OPCODE, invalidop);
    IDT_ALLOW_INTERRUPT(INT_DOUBLE_FAULT, doublefault);
    IDT_ALLOW_INTERRUPT(INT_INVALID_TSS, invalidtss);
    IDT_ALLOW_INTERRUPT(INT_SEGMENT_NOT_PRESENT, snp);
    IDT_ALLOW_INTERRUPT(INT_STACK_SEGMENT_NOT_PRESENT, ssnp);
    IDT_ALLOW_INTERRUPT(INT_GPF, gpf);
    IDT_ALLOW_INTERRUPT(INT_PAGE_FAULT, pagefault);
    IDT_ALLOW_INTERRUPT(INT_FLOATING_POINT, floatingpoint);
    
    idt_install(INT_DIV0, except_div0, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install(INT_NMI, except_nmi, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install(INT_OVERFLOW, except_overflow, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install(INT_BOUND_RANGE_EXCEEDED,
        except_bound_range_exceeded, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install(INT_INVALID_OPCODE, except_invalid_opcode, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install_with_error(INT_DOUBLE_FAULT, except_double_fault, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install_with_error(INT_INVALID_TSS, except_invalid_tss, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install_with_error(INT_SEGMENT_NOT_PRESENT,
        except_segment_not_present, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install_with_error(INT_STACK_SEGMENT_NOT_PRESENT,
        except_stack_segment_not_present, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install_with_error(INT_GPF, except_gpf, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install_with_error(INT_PAGE_FAULT, except_page_fault, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    idt_install(INT_FLOATING_POINT, except_floating_point, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
}
