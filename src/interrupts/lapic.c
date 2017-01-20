#include <stdint.h>

#include "interrupts/lapic.h"
#include "mem/page.h"
#include "main/printk.h"
#include "interrupts/exceptions.h"
#include "interrupts/wrapper.h"
#include "mem/gdt.h"

static volatile uint32_t *_base;

static uint32_t _read(uint32_t reg) {
    return _base[reg / sizeof(uint32_t)];
}

static void _write(uint32_t reg, uint32_t val) {
    _base[reg / sizeof(uint32_t)] = val;
}

void lapic_init() {
    page_t *page;
    
    page = page_create(LAPIC_BASE, PAGE_FLAG_KERNEL, 1);
    _base = page_kinstall(page, PAGE_TABLE_CACHEDISABLE | PAGE_TABLE_RW);
    
    printk("LAPIC ID: %x, Version: %x\n", _read(LAPIC_ID), _read(LAPIC_VER));
    // Set the spurious interrupt vector in order to get interrupts
    _write(LAPIC_SPURIOUS_INT_VECTOR, 0x1ff);
    
    // Set up the timer
    _write(LAPIC_TIMER_INITIAL, 0xffffff);
    _write(LAPIC_TIMER_DIVIDE_CONFIGURATION, 0x3);
    idt_install(EXCEPT_LAPIC_BASE, (uint32_t)int_wrap_lapic_timer, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    _write(LAPIC_LVT_TIMER, LAPIC_TIMER_MODE_PERIODIC | EXCEPT_LAPIC_BASE);
}

void lapic_timer(idt_proc_state_t state) {
    printk("Tick\n");
    _write(LAPIC_EOI, 0xffffffff);
}
