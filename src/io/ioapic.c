#include <stdint.h>

#include "io/utils.h"
#include "io/ioapic.h"
#include "io/ports.h"
#include "io/pic.h"
#include "main/printk.h"
#include "mem/page.h"
#include "interrupts/exceptions.h"
#include "interrupts/wrapper.h"
#include "mem/gdt.h"

#define _DATA 0x4 /* 4 bytes */
#define _ADDR 0x0

#define _REG_ID 0x0
#define _REG_VER 0x1
#define _REG_ARB 0x2
#define _REG_IRQ(x) ((x) * 2 + 0x10)

static volatile uint32_t *_base;

static void _write(uint32_t reg, uint32_t value) {
    _base[_ADDR] = reg;
    _base[_DATA] = value;
}

static uint32_t _read(uint32_t reg) {
    _base[_ADDR] = reg;
    return _base[_DATA];
}

void ioapic_init() {
    page_t *page;
    
    page = page_create(0xfec00000, PAGE_FLAG_KERNEL, 1);
    _base = page_kinstall(page, PAGE_TABLE_CACHEDISABLE | PAGE_TABLE_RW);
    
    printk("IOAPIC ID: %x, Version: %x (%x)\n", _read(_REG_ID), _read(_REG_VER), _read(_REG_IRQ(0)));
}

void ioapic_enable(uint8_t irq, uint8_t vector, uint64_t flags) {
    _write(_REG_IRQ(irq), vector | flags);
}

void ioapic_disable(uint8_t irq) {
    _write(_REG_IRQ(irq), IOAPIC_MASK);
}

void ioapic_enable_func(uint8_t irq, void (* func)(), uint64_t flags) {
    idt_install(EXCEPT_IOAPIC_BASE + irq, (uint32_t)func, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    ioapic_enable(irq, EXCEPT_IOAPIC_BASE + irq, flags);
}

void ioapic_keyboard(idt_proc_state_t state) {
    printk("Got keyboard input!\n");
}

#undef _DATA
#undef _ADDR

#undef _REG_ID
#undef _REG_VER
#undef _REG_ARB
#undef _REG_IRQ
