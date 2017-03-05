#ifndef _HPP_IO_SERIAL_
#define _HPP_IO_SERIAL_

#include "structures/stream.hpp"

namespace serial {
    class SerialPort : public stream::Stream {
    public:
        error_t write(const void *buff, size_t len, uint32_t flags, void *data, uint32_t *written);
        uint16_t io_port; // 0 if absent
    };

    class AllSerialPorts : public stream::Stream {
    public:
        error_t write(const void *buff, size_t len, uint32_t flags, void *data, uint32_t *written);
    };

    extern SerialPort serial_ports[4];
    extern AllSerialPorts all_serial_ports;

    extern "C" void serial_init();
}

#endif
