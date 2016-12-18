#include <stdint.h>

#include "interrupts/idt.h"
#include "mem/page.h"

#define _IDT_LENGTH 256

static volatile idt_entry_t table[_IDT_LENGTH];
static volatile idt_descriptor_t descriptor;

void idt_set_entry(idt_entry_t *entry, uint32_t offset, uint16_t selector, uint8_t type_attr) {
    entry->offset_low = (uint16_t)(offset & 0xffff);
    entry->selector = selector;
    entry->zero = 0;
    entry->type_attr = type_attr;
    entry->offset_high = (uint16_t)(offset >> 16);
}

void idt_install(uint8_t id, uint32_t offset, uint16_t selector, uint8_t type_attr) {
    idt_set_entry((idt_entry_t *)&table[id], offset, selector, type_attr | IDT_FLAG_PRESENT);
}

void idt_init() {
    int i;
    
    for(i = 0; i < _IDT_LENGTH; i ++) {
        idt_set_entry((idt_entry_t *)&table[i], 0, 0, 0);
    }
    
    descriptor.size = sizeof(table) - 1;
    descriptor.offset = ((uint32_t)&table);
    
    __asm__ volatile ("lidt (descriptor)");
}

#undef _IDT_LENGTH
