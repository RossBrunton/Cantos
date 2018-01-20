#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "main/loerror.hpp"
#include "main/common.hpp"

void low_error(const char *msg) {
    uint16_t *terminal_buffer = (uint16_t *)0xb8000;
    uint32_t i = 0;
    uint32_t p = 0;

    while("Error: "[i]) {
        terminal_buffer[i] = "Error: "[i] | (15 << 8);
        i ++;
    }

    while(msg[p]) {
        terminal_buffer[i++] = msg[p++] | (15 << 8);
    }

    while(1) {
        asm volatile ("cli");
        asm volatile ("hlt");
    };
}
