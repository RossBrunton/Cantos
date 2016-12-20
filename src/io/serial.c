#include <stdint.h>

#include "main/printk.h"
#include "io/serial.h"
#include "io/ports.h"
#include "io/utils.h"

#define _PORT_COUNT 4
#define _SCRATCH_TEST 0x53

serial_port_t serial_ports[_PORT_COUNT];
static uint16_t _check[] = {IO_PORT_COM1_BASE, IO_PORT_COM2_BASE, IO_PORT_COM3_BASE, IO_PORT_COM4_BASE};

int _transmit_empty(uint16_t base) {
   return inb(base + IO_PORT_COM_LINE_STATUS) & 0x20;
}

static int _serial_write(void *buff, size_t len, uint32_t flags, void *data, stream_t *stream) {
    (void)flags;
    (void)data;
    serial_port_t *port;
    size_t i = 0;
    
    port = stream->data;
    for(i = 0; i < len; i ++) {
        while(!_transmit_empty(port->io_port));
        outb(port->io_port, ((uint8_t *)buff)[i]);
    }
    return len;
}

static int _all_write(void *buff, size_t len, uint32_t flags, void *data, stream_t *stream) {
    (void)stream;
    int i;
    for(i = 0; i < _PORT_COUNT; i ++) {
        if(serial_ports[i].io_port) {
            stream_write(&serial_ports[i].stream, buff, len, flags, data);
        }
    }
    return len;
}

void serial_init() {
    int i;
    uint16_t base;
    serial_port_t *port;
    for(i = 0; i < _PORT_COUNT; i ++) {
        base = _check[i];
        
        outb(base + IO_PORT_COM_SCRATCH, _SCRATCH_TEST);
        io_wait();
        if(inb(base + IO_PORT_COM_SCRATCH) == _SCRATCH_TEST) {
#if DEBUG_SERIAL
            printk("Serial port on %d!\n", i);
#endif
            port = &serial_ports[i];
            
            port->io_port = base;
            
            outb(base + IO_PORT_COM_INT_ENABLE, 0x00); // Disable all interrupts
            outb(base + IO_PORT_COM_LINE_CONTROL, 0x80); // Enable DLAB (set baud rate divisor)
            outb(base + IO_PORT_COM_DAT, 0x03); // Set divisor to 3 (lo byte) 38400 baud
            outb(base + IO_PORT_COM_INT_ENABLE, 0x00); // (hi byte)
            outb(base + IO_PORT_COM_LINE_CONTROL, 0x03); // 8 bits, no parity, one stop bit
            outb(base + IO_PORT_COM_INT_ID, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
            outb(base + IO_PORT_COM_MODEM_CONTROL, 0x0B); // IRQs enabled, RTS/DSR set
            
            // Set up stream info
            port->stream.data = port;
            port->stream.write = _serial_write;
        }else{
#if DEBUG_SERIAL
            printk("Serial %d absent... (Got %d)\n", i, inb(base+IO_PORT_COM_SCRATCH));
#endif
        }
    }
}

stream_t serial_all_ports = {
    .write = _all_write
};

#undef _SCRATCH_TEST
#undef _PORT_COUNT
