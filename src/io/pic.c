#include <stdint.h>

#include "io/utils.h"
#include "io/pic.h"
#include "io/ports.h"
#include "main/printk.h"

void pic_init() {
    uint8_t mm;
    uint8_t sm;
    
    mm = ~(1 << IRQ_CASCADE);
    sm = ~0;
    
    outb(IO_PORT_MPIC_COM, PIC_ICW1_INIT + PIC_ICW1_ICW4);
    io_wait();
    outb(IO_PORT_SPIC_COM, PIC_ICW1_INIT + PIC_ICW1_ICW4);
    io_wait();
    outb(IO_PORT_MPIC_DAT, PIC_INT_MBASE);
    io_wait();
    outb(IO_PORT_SPIC_DAT, PIC_INT_SBASE);
    io_wait();
    outb(IO_PORT_MPIC_DAT, 4);
    io_wait();
    outb(IO_PORT_SPIC_DAT, 2);
    io_wait();
    
    outb(IO_PORT_MPIC_DAT, PIC_ICW4_8086);
    io_wait();
    outb(IO_PORT_SPIC_DAT, PIC_ICW4_8086);
    io_wait();
    
    outb(IO_PORT_MPIC_DAT, mm);
    outb(IO_PORT_SPIC_DAT, sm);
    
    __asm__ volatile ("sti");
}

void pic_enable(uint8_t irq) {
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

void pic_disable(uint8_t irq) {
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
