#include <stdint.h>

#include "mem/gdt.hpp"

namespace gdt {
    const uint8_t _LENGTH = 3;

    static volatile table_entry_t table[_LENGTH];
    extern "C" volatile descriptor_t gdt_descriptor = {};

    void set_entry(table_entry_t *entry, uint32_t base, uint32_t limit, uint8_t flags, uint8_t access) {
        uint64_t desc = 0;
        
        // Create the high 32 bit segment
        desc = limit & 0x000f0000; // Limit bits 16:19
        desc |= (flags << 20);
        desc |= (1 << 12);
        desc |= (access << 8);
        desc |= (base >> 16) & 0x000000ff; // Base bits 16:23
        desc |= base & 0xff000000; // Base bits 24:31
        
        // Shift by 32 to write the next word
        desc <<= 32;
        
        desc |= base << 16;
        desc |= limit & 0x0000ffff;
        
        entry->value = desc;
    }

    void init() {
        volatile table_entry_t *entry = table;
        
        // Null entry
        set_entry((table_entry_t *)entry, 0, 0, 0, ACCESS_PRESENT);
        entry ++;
        
        // Data
        set_entry((table_entry_t *)entry, 0, 0xfffff,
            FLAG_GRANULARITY | FLAG_SIZE,
            ACCESS_PRESENT | GDT_ACCESS_PRIV(0) | ACCESS_RW
        );
        entry ++;
        
        // Code
        set_entry((table_entry_t *)entry, 0, 0xfffff,
            FLAG_GRANULARITY | FLAG_SIZE,
            ACCESS_PRESENT | GDT_ACCESS_PRIV(0) | ACCESS_EXECUTABLE | ACCESS_DC | ACCESS_RW
        );
        
        gdt_descriptor.size = sizeof(table_entry_t) * _LENGTH;
        gdt_descriptor.offset = (uint32_t)table;
    }

    void setup() {
        asm volatile ("lgdt (gdt_descriptor)");
        asm volatile goto ("ljmp $0x10, $%l0" :::: next);
        next:
        asm volatile ("mov $0x8, %ax");
        asm volatile ("mov %ax, %ds");
        asm volatile ("mov %ax, %es");
        asm volatile ("mov %ax, %fs");
        asm volatile ("mov %ax, %gs");
        asm volatile ("mov %ax, %ss");
    }
}
