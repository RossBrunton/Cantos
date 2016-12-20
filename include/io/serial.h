#ifndef _H_IO_SERIAL_
#define _H_IO_SERIAL_

#include "main/stream.h"

typedef struct serial_port_s {
    uint16_t io_port; // Set to 0 if absent
    stream_t stream;
} serial_port_t;

extern serial_port_t serial_ports[4];
extern stream_t serial_all_ports;

void serial_init();

#endif
