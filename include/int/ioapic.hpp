#ifndef _HPP_IO_IOAPIC_
#define _HPP_IO_IOAPIC_

#include "int/idt.hpp"

namespace ioapic {
    const uint32_t MODE_FIXED = (0x0 << 8);
    const uint32_t MODE_LOWEST = (0x1 << 8);
    const uint32_t MODE_SMI = (0x10 << 8);
    const uint32_t MODE_NMI = (0x100 << 8);
    const uint32_t MODE_INIT = (0x101 << 8);
    const uint32_t MODE_EXTINIT = (0x111 << 8);

    const uint32_t DEST_PHYSICAL = (0x0 << 11);
    const uint32_t DEST_LOGICAL = (0x1 << 11);

    const uint32_t ACTIVE_HIGH = (0x0 << 13);
    const uint32_t ACTIVE_LOW = (0x1 << 13);

    const uint32_t TRIGGER_EDGE = (0x0 << 15);
    const uint32_t TRIGGER_LEVEL = (0x1 << 15);

    const uint32_t MASK = (0x1 << 16);

    void init();

    void enable(uint8_t irq, uint8_t vector, uint64_t flags);
    void disable(uint8_t irq);

    void enable_func(uint8_t irq, idt::interrupt_handler_t func, uint64_t flags);

    void keyboard(idt_proc_state_t state);
}

#endif
