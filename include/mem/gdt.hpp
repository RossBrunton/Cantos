#ifndef _HPP_MEM_GDT_
#define _HPP_MEM_GDT_

#include <stdint.h>

namespace gdt {
    struct __attribute__((packed)) descriptor_t {
        uint16_t size;
        uint32_t offset;
    };

    struct table_entry_t {
        uint64_t value;
    };

    const uint8_t ACCESS_PRESENT = (1<<7);
    #define GDT_ACCESS_PRIV(x) ((x)<<5)
    const uint8_t ACCESS_EXECUTABLE = (1<<3);
    const uint8_t ACCESS_DC = (1<<2);
    const uint8_t ACCESS_RW = (1<<1);

    const uint8_t FLAG_GRANULARITY = (1<<3);
    const uint8_t FLAG_SIZE = (1<<2);

    const uint8_t CODE_OFFSET = 0x10;
    const uint8_t DATA_OFFSET = 0x08;

    #define GDT_SELECTOR(rpl, ti, index) ((rpl) | ((ti) << 2) | ((index) << 3))

    void set_entry(table_entry_t *entry, uint32_t base, uint32_t limit, uint8_t flags, uint8_t access);
    void init();
    void setup();

    extern "C" volatile descriptor_t gdt_descriptor;
}

#endif
