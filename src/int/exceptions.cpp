#include <stdint.h>

#include "int/exceptions.hpp"
#include "main/panic.hpp"
#include "int/idt.hpp"
#include "mem/gdt.hpp"
#include "main/cpu.hpp"

extern "C" {
    #include "int/numbers.h"
    #include "main/common.h"
}

namespace exceptions {
    void div0(idt_proc_state_t state) {
        uint32_t eip = *(uint32_t *)(state.esp);
        panic_at(state.ebp, eip, "Division by 0");
    }

    void debug(idt_proc_state_t state) {
        uint32_t eip = *(uint32_t *)(state.esp);
        panic_at(state.ebp, eip, "Debug Exception");
    }

    void nmi(idt_proc_state_t state) {
        uint32_t eip = *(uint32_t *)(state.esp);
        panic_at(state.ebp, eip, "NMI Received (Hardware Failure?)");
    }

    void overflow(idt_proc_state_t state) {
        uint32_t eip = *(uint32_t *)(state.esp);
        panic_at(state.ebp, eip, "Overflow Exception");
    }

    void bound_range_exceeded(idt_proc_state_t state) {
        uint32_t eip = *(uint32_t *)(state.esp);
        panic_at(state.ebp, eip, "Bound Range Exceeded Exception");
    }

    void invalid_opcode(idt_proc_state_t state) {
        uint32_t eip = *(uint32_t *)(state.esp);
        panic_at(state.ebp, eip, "Invalid Opcode");
    }

    void double_fault(idt_proc_state_t state, uint32_t errcode) {
        uint32_t eip = *(uint32_t *)(state.esp + 4);
        panic_at(state.ebp, eip, "Double Fault %x", errcode);
    }

    void invalid_tss(idt_proc_state_t state, uint32_t errcode) {
        uint32_t eip = *(uint32_t *)(state.esp + 4);
        panic_at(state.ebp, eip, "Invalid TSS %x", errcode);
    }

    void segment_not_present(idt_proc_state_t state, uint32_t errcode) {
        uint32_t eip = *(uint32_t *)(state.esp + 4);
        panic_at(state.ebp, eip, "Segment not Present %x", errcode);
    }

    void stack_segment_not_present(idt_proc_state_t state, uint32_t errcode) {
        uint32_t eip = *(uint32_t *)(state.esp + 4);
        (void)errcode;
        panic_at(state.ebp, eip, "Stack-Segment not Present %x", errcode);
    }

    void gpf(idt_proc_state_t state, uint32_t errcode) {
        uint32_t eip = *(uint32_t *)(state.esp + 4);
        panic_at(state.ebp, eip, "General Protection Fault %x", errcode);
    }

    void page_fault(idt_proc_state_t state, uint32_t errcode) {
        uint32_t eip = *(uint32_t *)(state.esp + 4);
        uint32_t addr;
        __asm__("mov %%cr2, %0" : "=r"(addr));

        if(!(cpu::current_thread() && cpu::current_thread()->vm->resolve_fault(addr))) {
            panic_at(state.ebp, eip, "Unresolved Page Fault %x [Address: %p]", errcode, addr);
        }
    }

    void floating_point(idt_proc_state_t state) {
        uint32_t eip = *(uint32_t *)(state.esp);
        panic_at(state.ebp, eip, "Floating Point Exception");
    }

    IDT_TELL_INTERRUPT(div0);
    IDT_TELL_INTERRUPT(debug);
    IDT_TELL_INTERRUPT(nmi);
    IDT_TELL_INTERRUPT(overflow);
    IDT_TELL_INTERRUPT(bre);
    IDT_TELL_INTERRUPT(invalidop);
    IDT_TELL_INTERRUPT(doublefault);
    IDT_TELL_INTERRUPT(invalidtss);
    IDT_TELL_INTERRUPT(snp);
    IDT_TELL_INTERRUPT(ssnp);
    IDT_TELL_INTERRUPT(gpf);
    IDT_TELL_INTERRUPT(pagefault);
    IDT_TELL_INTERRUPT(floatingpoint);

    void init() {
        IDT_ALLOW_INTERRUPT(INT_DIV0, div0);
        IDT_ALLOW_INTERRUPT(INT_DEBUG, debug);
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
        
        idt::install(INT_DIV0, div0, GDT_SELECTOR(0, 0, 2), idt::GATE_32_INT);
        idt::install(INT_DEBUG, debug, GDT_SELECTOR(0, 0, 2), idt::GATE_32_INT);
        idt::install(INT_NMI, nmi, GDT_SELECTOR(0, 0, 2), idt::GATE_32_INT);
        idt::install(INT_OVERFLOW, overflow, GDT_SELECTOR(0, 0, 2), idt::GATE_32_INT);
        idt::install(INT_BOUND_RANGE_EXCEEDED,
            bound_range_exceeded, GDT_SELECTOR(0, 0, 2), idt::GATE_32_INT);
        idt::install(INT_INVALID_OPCODE, invalid_opcode, GDT_SELECTOR(0, 0, 2), idt::GATE_32_INT);
        idt::install_with_error(INT_DOUBLE_FAULT, double_fault, GDT_SELECTOR(0, 0, 2), idt::GATE_32_INT);
        idt::install_with_error(INT_INVALID_TSS, invalid_tss, GDT_SELECTOR(0, 0, 2), idt::GATE_32_INT);
        idt::install_with_error(INT_SEGMENT_NOT_PRESENT,
            segment_not_present, GDT_SELECTOR(0, 0, 2), idt::GATE_32_INT);
        idt::install_with_error(INT_STACK_SEGMENT_NOT_PRESENT,
            stack_segment_not_present, GDT_SELECTOR(0, 0, 2), idt::GATE_32_INT);
        idt::install_with_error(INT_GPF, gpf, GDT_SELECTOR(0, 0, 2), idt::GATE_32_INT);
        idt::install_with_error(INT_PAGE_FAULT, page_fault, GDT_SELECTOR(0, 0, 2), idt::GATE_32_INT);
        idt::install(INT_FLOATING_POINT, floating_point, GDT_SELECTOR(0, 0, 2), idt::GATE_32_INT);
    }
}
