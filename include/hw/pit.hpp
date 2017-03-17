#ifndef _HPP_INTERRUPTS_PIT_
#define _HPP_INTERRUPTS_PIT_

#include "main/common.h"

namespace pit {
    const uint8_t ACCESS_LATCH = 0;
    const uint8_t ACCESS_LOW = 1;
    const uint8_t ACCESS_HIGH = 2;
    const uint8_t ACCESS_LOWHIGH = 3;

    const uint8_t MODE_INT_ON_TERMINAL = 0;
    const uint8_t MODE_ONE_SHOT = 1;
    const uint8_t MODE_RATE_GEN = 2;
    const uint8_t MODE_SQUARE_WAVE = 3;
    const uint8_t MODE_SOFTWARE_STROBE = 4;
    const uint8_t MODE_HARDWARE_STROBE = 5;

    const uint64_t FREQUENCY = 1193182;
    const uint32_t DIVISOR = 14551;
    const uint32_t PER_SECOND = 82;

    void init();
    void interrupt(idt_proc_state_t state);
    extern volatile uint32_t time;
}

#endif
