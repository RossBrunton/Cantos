#include <stdint.h>

#include "hw/utils.h"
#include "int/ioapic.h"
#include "hw/ports.h"
#include "int/pic.h"
#include "main/printk.h"
#include "mem/page.h"
#include "int/exceptions.h"
#include "int/numbers.h"
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
    
    IDT_ALLOW_INTERRUPT(INT_IOAPIC_BASE + INT_IRQ_KEYBOARD, iokeyboard);
    IDT_ALLOW_INTERRUPT(INT_IOAPIC_BASE + INT_IRQ_INTERRUPT_TIMER_IOAPIC, iotimer);
    IDT_ALLOW_INTERRUPT(INT_IOAPIC_BASE + INT_IRQ_COM2, iocom1);
    IDT_ALLOW_INTERRUPT(INT_IOAPIC_BASE + INT_IRQ_COM1, iocom2);
    IDT_ALLOW_INTERRUPT(INT_IOAPIC_BASE + INT_IRQ_LPT2, iolpt2);
    IDT_ALLOW_INTERRUPT(INT_IOAPIC_BASE + INT_IRQ_FLOPPY, iofloppy);
    IDT_ALLOW_INTERRUPT(INT_IOAPIC_BASE + INT_IRQ_LPT1, iolpt1);
    IDT_ALLOW_INTERRUPT(INT_IOAPIC_BASE + INT_IRQ_CMOS, iocmos);
    IDT_ALLOW_INTERRUPT(INT_IOAPIC_BASE + INT_IRQ_9, io9);
    IDT_ALLOW_INTERRUPT(INT_IOAPIC_BASE + INT_IRQ_10, io10);
    IDT_ALLOW_INTERRUPT(INT_IOAPIC_BASE + INT_IRQ_11, io11);
    IDT_ALLOW_INTERRUPT(INT_IOAPIC_BASE + INT_IRQ_MOUSE, iomouse);
    IDT_ALLOW_INTERRUPT(INT_IOAPIC_BASE + INT_IRQ_FPU, iofpu);
    IDT_ALLOW_INTERRUPT(INT_IOAPIC_BASE + INT_IRQ_PATA, iopata);
    IDT_ALLOW_INTERRUPT(INT_IOAPIC_BASE + INT_IRQ_SATA, iosata);
    
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

void ioapic_enable_func(uint8_t irq, idt_interrupt_handler_t func, uint64_t flags) {
    idt_install(INT_IOAPIC_BASE + irq, func, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    ioapic_enable(irq, INT_IOAPIC_BASE + irq, flags);
}

void ioapic_keyboard(idt_proc_state_t state) {
    (void)state;
    printk("Got keyboard input!\n");
}

#undef _DATA
#undef _ADDR

#undef _REG_ID
#undef _REG_VER
#undef _REG_ARB
#undef _REG_IRQ
