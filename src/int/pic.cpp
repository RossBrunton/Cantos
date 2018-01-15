#include <stdint.h>

#include "hw/utils.h"
#include "int/pic.hpp"
#include "hw/ports.h"
#include "main/printk.hpp"
#include "main/common.hpp"
#include "int/numbers.h"

namespace pic {
    void init() {
        uint8_t mm;
        uint8_t sm;

        mm = ~(1 << INT_IRQ_CASCADE);
        sm = ~0;

        outb(IO_PORT_MPIC_COM, ICW1_INIT + ICW1_ICW4);
        io_wait();
        outb(IO_PORT_SPIC_COM, ICW1_INIT + ICW1_ICW4);
        io_wait();
        outb(IO_PORT_MPIC_DAT, INT_MBASE);
        io_wait();
        outb(IO_PORT_SPIC_DAT, INT_SBASE);
        io_wait();
        outb(IO_PORT_MPIC_DAT, 4);
        io_wait();
        outb(IO_PORT_SPIC_DAT, 2);
        io_wait();

        outb(IO_PORT_MPIC_DAT, ICW4_8086);
        io_wait();
        outb(IO_PORT_SPIC_DAT, ICW4_8086);
        io_wait();

        outb(IO_PORT_MPIC_DAT, mm);
        outb(IO_PORT_SPIC_DAT, sm);

        __asm__ volatile ("sti");
    }

    void enable(uint8_t irq) {
        uint16_t port;
        uint8_t value;

        if(irq < 8) {
            port = IO_PORT_MPIC_DAT;
        }else{
            port = IO_PORT_SPIC_DAT;
            irq -= 8;
        }

        value = inb(port) & ~(1 << irq);
        outb(port, value);
    }

    void disable(uint8_t irq) {
        uint16_t port;
        uint8_t value;

        if(irq < 8) {
            port = IO_PORT_MPIC_DAT;
        }else{
            port = IO_PORT_SPIC_DAT;
            irq -= 8;
        }

        value = inb(port) | (1 << irq);
        outb(port, value);
    }
}
