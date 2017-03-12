#ifndef _H_IO_IOAPIC_
#define _H_IO_IOAPIC_

#include "int/idt.h"

void ioapic_init();

#define IOAPIC_MODE_FIXED (0x0 << 8)
#define IOAPIC_MODE_LOWEST (0x1 << 8)
#define IOAPIC_MODE_SMI (0x10 << 8)
#define IOAPIC_MODE_NMI (0x100 << 8)
#define IOAPIC_MODE_INIT (0x101 << 8)
#define IOAPIC_MODE_EXTINIT (0x111 << 8)

#define IOAPIC_DEST_PHYSICAL (0x0 << 11)
#define IOAPIC_DEST_LOGICAL (0x1 << 11)

#define IOAPIC_ACTIVE_HIGH (0x0 << 13)
#define IOAPIC_ACTIVE_LOW (0x1 << 13)

#define IOAPIC_TRIGGER_EDGE (0x0 << 15)
#define IOAPIC_TRIGGER_LEVEL (0x1 << 15)

#define IOAPIC_MASK (0x1 << 16)

void ioapic_enable(uint8_t irq, uint8_t vector, uint64_t flags);
void ioapic_disable(uint8_t irq);

void ioapic_enable_func(uint8_t irq, idt_interrupt_handler_t func, uint64_t flags);

void ioapic_keyboard(idt_proc_state_t state);

#endif
