#include <stdint.h>

#include "io/pit.h"
#include "io/ports.h"
#include "main/printk.h"
#include "io/utils.h"
#include "io/ioapic.h"
#include "interrupts/wrapper.h"
#include "interrupts/lapic.h"

volatile int pit_time = 0;
static int seconds = 0;

static void _set_mode(uint8_t channel, uint8_t access_mode, uint8_t operating_mode) {
    uint8_t hold = 0;
    hold |= channel << 6;
    hold |= access_mode << 4;
    hold |= operating_mode << 1;
    
    outb(IO_PORT_PIT_MODE, hold);
}

void pit_init() {
    ioapic_enable_func(IRQ_INTERRUPT_TIMER_IOAPIC, int_wrap_io_pit, 0);
    _set_mode(0, PIT_ACCESS_LOWHIGH, PIT_MODE_RATE_GEN);
    outb(IO_PORT_PIT_0, PIT_DIVISOR && 0xff);
    outb(IO_PORT_PIT_0, PIT_DIVISOR >> 8);
    printk("Initing timer!\n");
}

void pit_interrupt(idt_proc_state_t state) {
    pit_time ++;
    
    if(!(pit_time % PIT_PER_SECOND)) {
        seconds ++;
        printk("PIT fire! %d\n", seconds);
    }
    lapic_eoi();
}
