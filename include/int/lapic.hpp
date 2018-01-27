#ifndef _HPP_INTERRUPTS_LAPIC_
#define _HPP_INTERRUPTS_LAPIC_

#include <stdint.h>

#include "main/common.hpp"

namespace lapic {
    const uint32_t ID = 0x20;
    const uint32_t VER = 0x30;
    const uint32_t TPR = 0x80;
    const uint32_t APR = 0x90;
    const uint32_t PPR = 0xa0;
    const uint32_t EOI = 0xb0;
    const uint32_t LOGICAL_DEST = 0xd0;
    const uint32_t DESTINATION_FORMAT = 0xe0;
    const uint32_t SPURIOUS_INT_VECTOR = 0xf0;
    const uint32_t ISR_BASE = 0x100;
    const uint32_t TMR_BASE = 0x180;
    const uint32_t IRR_BASE = 0x200;
    const uint32_t ERROR_STATUS = 0x280;
    const uint32_t ICR_A = 0x300;
    const uint32_t ICR_B = 0x310;
    const uint32_t LVT_TIMER = 0x320;
    const uint32_t LVT_THERMAL = 0x330;
    const uint32_t LVT_PERFORMANCE = 0x340;
    const uint32_t LVT_LINT0 = 0x350;
    const uint32_t LVT_LINT1 = 0x350;
    const uint32_t LVT_ERROR = 0x360;
    const uint32_t TIMER_INITIAL = 0x380;
    const uint32_t TIMER_CURRENT = 0x390;
    const uint32_t TIMER_DIVIDE_CONFIGURATION = 0x3e0;

    const uint32_t LVT_MASK = (1 << 16);
    const uint32_t TIMER_MODE_PERIODIC = (1 << 17);
    const uint32_t TIMER_MODE_ONE_SHOT = (0 << 17);

    const uint32_t SWITCHES_PER_SECOND = 1000;

    enum command_t {
        CMD_INVLPG
    };

    void init();
    void setup();
    void timer(idt_proc_state_t state);
    void handle_command(idt_proc_state_t state);
    void handle_panic(idt_proc_state_t state);
    void eoi();
    void awaken_others();

    void ipi(uint8_t vector, uint32_t proc);
    void ipi_all(uint8_t vector);

    void send_command(command_t command, uint32_t argument, uint32_t proc);
    void send_command_all(command_t command, uint32_t argument);
}

#endif
