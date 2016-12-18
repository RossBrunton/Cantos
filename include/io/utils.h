#ifndef _H_IO_UTILS
#define _H_IO_UTILS

#include <stdint.h>

void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
void io_wait();

#endif
