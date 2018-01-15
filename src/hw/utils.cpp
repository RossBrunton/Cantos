#include "hw/utils.h"
#include "hw/ports.h"

#include <stdint.h>

void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outl(uint16_t port, uint32_t val) {
    __asm__ volatile ("out %0, %1" : : "a"(val), "d"(port));
}

uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile ("in %1, %0" : "=a"(ret) : "d"(port));
    return ret;
}

void io_wait() {
    outb(IO_PORT_CHECKPOINT, 0);
}
