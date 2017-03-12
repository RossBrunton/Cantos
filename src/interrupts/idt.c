#include <stdint.h>

#include "interrupts/idt.h"
#include "mem/page.h"
#include "main/common.h"
#include "main/panic.h"

#define _IDT_LENGTH 256
#define _INT(x) idt_asm_interrupt_ ## x

static volatile idt_entry_t table[_IDT_LENGTH];
static volatile idt_interrupt_handler_t functions[_IDT_LENGTH];
static volatile idt_interrupt_handler_err_t functions_err[_IDT_LENGTH];
static volatile idt_descriptor_t descriptor;

void idt_enable_entry(uint8_t vector, uint32_t offset) {
    table[vector].offset_low = (uint16_t)(offset & 0xffff);
    table[vector].offset_high = (uint16_t)(offset >> 16);
}

void idt_update_entry(idt_entry_t *entry, uint16_t selector, uint8_t type_attr) {
    entry->selector = selector;
    entry->zero = 0;
    entry->type_attr = type_attr;
}

void idt_install(uint8_t id, idt_interrupt_handler_t offset, uint16_t selector, uint8_t type_attr) {
    functions[id] = offset;
    if(table[id].offset_high == 0) {
        panic("Tried to install an IDT entry for un-enabled interrupt 0x%x", id);
    }
    idt_update_entry((idt_entry_t *)&table[id], selector, type_attr | IDT_FLAG_PRESENT);
}

void idt_install_with_error(uint8_t id, idt_interrupt_handler_err_t offset, uint16_t selector, uint8_t type_attr) {
    functions_err[id] = offset;
    if(table[id].offset_high == 0) {
        panic("Tried to install an IDT entry for un-enabled interrupt 0x%x", id);
    }
    idt_update_entry((idt_entry_t *)&table[id], selector, type_attr | IDT_FLAG_PRESENT);
}

void idt_init() {
    int i;
    
    /*for(i = 0; i < _IDT_LENGTH; i ++) {
        idt_update_entry((idt_entry_t *)&table[i], 0, 0);
    }*/
    
    descriptor.size = sizeof(table) - 1;
    descriptor.offset = ((uint32_t)&table);
    
    __asm__ volatile ("lidt (descriptor)");
}

void idt_handle(uint32_t vector, idt_proc_state_t state) {
    if(functions[vector]) {
        functions[vector](state);
    }
}

void idt_handle_with_error(uint32_t vector, idt_proc_state_t state, uint32_t errcode) {
    if(functions_err[vector]) {
        functions_err[vector](state, errcode);
    }
}

#undef _IDT_LENGTH
#undef _INT
