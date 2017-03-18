#include <stdint.h>

#include "int/idt.hpp"
#include "mem/page.hpp"
#include "main/panic.hpp"

extern "C" {
#include "main/common.h"
}

namespace idt {
    const uint32_t _IDT_LENGTH = 256;

    static volatile entry_t table[_IDT_LENGTH];
    static volatile interrupt_handler_t functions[_IDT_LENGTH];
    static volatile interrupt_handler_err_t functions_err[_IDT_LENGTH];
    extern "C" volatile descriptor_t idt_descriptor = {};

    void enable_entry(uint8_t vector, uint32_t offset) {
        table[vector].offset_low = (uint16_t)(offset & 0xffff);
        table[vector].offset_high = (uint16_t)(offset >> 16);
    }

    void update_entry(entry_t *entry, uint16_t selector, uint8_t type_attr) {
        entry->selector = selector;
        entry->zero = 0;
        entry->type_attr = type_attr;
    }

    void install(uint8_t id, interrupt_handler_t offset, uint16_t selector, uint8_t type_attr) {
        functions[id] = offset;
        if(table[id].offset_high == 0) {
            panic("Tried to install an IDT entry for un-enabled interrupt 0x%x", id);
        }
        update_entry((entry_t *)&table[id], selector, type_attr | FLAG_PRESENT);
    }

    void install_with_error(uint8_t id, interrupt_handler_err_t offset, uint16_t selector, uint8_t type_attr) {
        functions_err[id] = offset;
        if(table[id].offset_high == 0) {
            panic("Tried to install an IDT entry for un-enabled interrupt 0x%x", id);
        }
        update_entry((entry_t *)&table[id], selector, type_attr | FLAG_PRESENT);
    }

    void init() {
        int i;
        
        /*for(i = 0; i < _IDT_LENGTH; i ++) {
            update_entry((entry_t *)&table[i], 0, 0);
        }*/
        
        idt_descriptor.size = sizeof(table) - 1;
        idt_descriptor.offset = ((uint32_t)&table);
    }

    void setup() {
        asm volatile ("lidt (idt_descriptor)");
    }

    extern "C" void idt_handle(uint32_t vector, idt_proc_state_t state) {
        if(functions[vector]) {
            functions[vector](state);
        }
    }

    extern "C" void idt_handle_with_error(uint32_t vector, idt_proc_state_t state, uint32_t errcode) {
        if(functions_err[vector]) {
            functions_err[vector](state, errcode);
        }
    }
}
