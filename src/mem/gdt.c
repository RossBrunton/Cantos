#include <stdint.h>

#include "mem/gdt.h"

#define _GDT_LENGTH 3

static volatile gdt_table_entry_t table[_GDT_LENGTH];
volatile gdt_descriptor_t gdt_descriptor;

void gdt_set_entry(gdt_table_entry_t *entry, uint32_t base, uint32_t limit, uint8_t flags, uint8_t access) {
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

void gdt_init() {
    volatile gdt_table_entry_t *entry = table;
    
    // Null entry
    gdt_set_entry((gdt_table_entry_t *)entry, 0, 0, 0, GDT_ACCESS_PRESENT);
    entry ++;
    
    // Data
    gdt_set_entry((gdt_table_entry_t *)entry, 0, 0xfffff,
        GDT_FLAG_GRANULARITY | GDT_FLAG_SIZE,
        GDT_ACCESS_PRESENT | GDT_ACCESS_PRIV(0) | GDT_ACCESS_RW
    );
    entry ++;
    
    // Code
    gdt_set_entry((gdt_table_entry_t *)entry, 0, 0xfffff,
        GDT_FLAG_GRANULARITY | GDT_FLAG_SIZE,
        GDT_ACCESS_PRESENT | GDT_ACCESS_PRIV(0) | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_DC | GDT_ACCESS_RW
    );
    
    gdt_descriptor.size = sizeof(gdt_table_entry_t) * _GDT_LENGTH;
    gdt_descriptor.offset = (uint32_t)table;
}

void gdt_setup() {
    __asm__ volatile ("lgdt (gdt_descriptor)");
    __asm__ volatile goto ("ljmp $0x10, $%l0" :::: next);
    next:
    __asm__ volatile ("mov $0x8, %ax");
    __asm__ volatile ("mov %ax, %ds");
    __asm__ volatile ("mov %ax, %es");
    __asm__ volatile ("mov %ax, %fs");
    __asm__ volatile ("mov %ax, %gs");
    __asm__ volatile ("mov %ax, %ss");
}

#undef _GDT_LENGTH
