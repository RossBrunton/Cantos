#include <stdint.h>

#include "io/serial.hpp"
#include "main/printk.hpp"
#include "io/ports.h"
#include "io/utils.h"

namespace serial {
    const int _PORT_COUNT = 4;
    const int _SCRATCH_TEST = 0x53;

    SerialPort serial_ports[_PORT_COUNT];
    AllSerialPorts all_serial_ports;
    static uint16_t _check[] = {IO_PORT_COM1_BASE, IO_PORT_COM2_BASE, IO_PORT_COM3_BASE, IO_PORT_COM4_BASE};

    int _transmit_empty(uint16_t base) {
       return inb(base + IO_PORT_COM_LINE_STATUS) & 0x20;
    }

    error_t SerialPort::write(const void *buff, size_t len, uint32_t flags, void *data, uint32_t *written) {
        size_t i = 0;

        for(i = 0; i < len; i ++) {
            while(!_transmit_empty(this->io_port));
            outb(this->io_port, ((uint8_t *)buff)[i]);
        }

        *written = len;
        return EOK;
    }

    error_t AllSerialPorts::write(const void *buff, size_t len, uint32_t flags, void *data, uint32_t *written) {
        int i;
        for(i = 0; i < _PORT_COUNT; i ++) {
            if(serial_ports[i].io_port) {
                serial_ports[i].write(buff, len, flags, data, written);
            }
        }

        *written = len;
        return EOK;
    }

    extern "C" void serial_init() {
        int i;
        uint16_t base;
        SerialPort *port;
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
            }else{
#if DEBUG_SERIAL
                printk("Serial %d absent... (Got %d)\n", i, inb(base+IO_PORT_COM_SCRATCH));
#endif
            }
        }
    }

}
