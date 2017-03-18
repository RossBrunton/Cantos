#include <stdint.h>

#include "int/ioapic.hpp"

#include "main/printk.hpp"
#include "int/idt.hpp"
#include "mem/gdt.hpp"
#include "mem/page.hpp"

extern "C" {
#include "hw/utils.h"
#include "hw/ports.h"
#include "int/numbers.h"
}

namespace ioapic {
    const uint8_t _DATA = 0x4 /* 4 bytes */;
    const uint8_t _ADDR = 0x0;

    const uint8_t _REG_ID = 0x0;
    const uint8_t _REG_VER = 0x1;
    const uint8_t _REG_ARB = 0x2;
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

    IDT_TELL_INTERRUPT(iokeyboard);
    IDT_TELL_INTERRUPT(iotimer);
    IDT_TELL_INTERRUPT(iocom1);
    IDT_TELL_INTERRUPT(iocom2);
    IDT_TELL_INTERRUPT(iolpt2);
    IDT_TELL_INTERRUPT(iofloppy);
    IDT_TELL_INTERRUPT(iolpt1);
    IDT_TELL_INTERRUPT(iocmos);
    IDT_TELL_INTERRUPT(io9);
    IDT_TELL_INTERRUPT(io10);
    IDT_TELL_INTERRUPT(io11);
    IDT_TELL_INTERRUPT(iomouse);
    IDT_TELL_INTERRUPT(iofpu);
    IDT_TELL_INTERRUPT(iopata);
    IDT_TELL_INTERRUPT(iosata);

    void init() {
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
        _base = (uint32_t *)page_kinstall(page, PAGE_TABLE_CACHEDISABLE | PAGE_TABLE_RW);

        printk("IOAPIC ID: %x, Version: %x (%x)\n", _read(_REG_ID), _read(_REG_VER), _read(_REG_IRQ(0)));
    }

    void enable(uint8_t irq, uint8_t vector, uint64_t flags) {
        _write(_REG_IRQ(irq), vector | flags);
    }

    void disable(uint8_t irq) {
        _write(_REG_IRQ(irq), MASK);
    }

    void enable_func(uint8_t irq, idt::interrupt_handler_t func, uint64_t flags) {
        idt::install(INT_IOAPIC_BASE + irq, func, GDT_SELECTOR(0, 0, 2), idt::GATE_32_INT);
        enable(irq, INT_IOAPIC_BASE + irq, flags);
    }

    void keyboard(idt_proc_state_t state) {
        (void)state;
        printk("Got keyboard input!\n");
    }

    #undef _REG_IRQ
}
