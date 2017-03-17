#include <stdint.h>

#include "hw/pit.hpp"
#include "main/printk.hpp"
#include "int/ioapic.hpp"

extern "C" {
    #include "hw/ports.h"
    #include "hw/utils.h"
    #include "int/lapic.h"
    #include "int/numbers.h"
}

namespace pit {
    volatile uint32_t time = 0;

    static void _set_mode(uint8_t channel, uint8_t access_mode, uint8_t operating_mode) {
        uint8_t hold = 0;
        hold |= channel << 6;
        hold |= access_mode << 4;
        hold |= operating_mode << 1;
        
        outb(IO_PORT_PIT_MODE, hold);
    }

    void init() {
        ioapic::enable_func(INT_IRQ_INTERRUPT_TIMER_IOAPIC, interrupt, 0);
        _set_mode(0, ACCESS_LOWHIGH, MODE_RATE_GEN);
        outb(IO_PORT_PIT_0, DIVISOR && 0xff);
        outb(IO_PORT_PIT_0, DIVISOR >> 8);
        printk("Initing timer!\n");
    }

    void interrupt(idt_proc_state_t state) {
        (void)state;
        time ++;
        
        lapic_eoi();
    }
}
