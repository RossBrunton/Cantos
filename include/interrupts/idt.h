#ifndef _H_INTERRUPTS_IDT_
#define _H_INTERRUPTS_IDT_

#include <stdint.h>

typedef struct idt_proc_state_s {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
} idt_proc_state_t;

typedef struct __attribute__((packed)) idt_descriptor_s {
    uint16_t size;
    uint32_t offset;
} idt_descriptor_t;

typedef struct idt_entry_s {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
} idt_entry_t;

#define IDT_FLAG_PRESENT (1 << 7)
#define IDT_FLAG_DPL(x) ((x) << 5)

#define IDT_GATE_32_TASK 0x5
#define IDT_GATE_16_INT 0x6
#define IDT_GATE_16_TRAP 0x7
#define IDT_GATE_32_INT 0xe
#define IDT_GATE_32_TRAP 0xf

void idt_set_entry(idt_entry_t *entry, uint32_t offset, uint16_t selector, uint8_t type_attr);
void idt_install(uint8_t id, uint32_t offset, uint16_t selector, uint8_t type_attr);
void idt_init();

#endif
