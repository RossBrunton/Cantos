#include <stdint.h>

#include "hw/pit.h"
#include "hw/ports.h"
#include "main/printk.h"
#include "hw/utils.h"
#include "int/ioapic.h"
#include "int/lapic.h"
#include "int/numbers.h"

volatile uint32_t pit_time = 0;

static void _set_mode(uint8_t channel, uint8_t access_mode, uint8_t operating_mode) {
    uint8_t hold = 0;
    hold |= channel << 6;
    hold |= access_mode << 4;
    hold |= operating_mode << 1;
    
    outb(IO_PORT_PIT_MODE, hold);
}

void pit_init() {
    ioapic_enable_func(INT_IRQ_INTERRUPT_TIMER_IOAPIC, pit_interrupt, 0);
    _set_mode(0, PIT_ACCESS_LOWHIGH, PIT_MODE_RATE_GEN);
    outb(IO_PORT_PIT_0, PIT_DIVISOR && 0xff);
    outb(IO_PORT_PIT_0, PIT_DIVISOR >> 8);
    printk("Initing timer!\n");
}

void pit_interrupt(idt_proc_state_t state) {
    (void)state;
    pit_time ++;
    
    lapic_eoi();
}
