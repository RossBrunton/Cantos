#ifndef _H_INTERRUPTS_PIT_
#define _H_INTERRUPTS_PIT_

#include "main/common.h"

#define PIT_ACCESS_LATCH 0
#define PIT_ACCESS_LOW 1
#define PIT_ACCESS_HIGH 2
#define PIT_ACCESS_LOWHIGH 3

#define PIT_MODE_INT_ON_TERMINAL 0
#define PIT_MODE_ONE_SHOT 1
#define PIT_MODE_RATE_GEN 2
#define PIT_MODE_SQUARE_WAVE 3
#define PIT_MODE_SOFTWARE_STROBE 4
#define PIT_MODE_HARDWARE_STROBE 5

#define PIT_FREQUENCY 1193182ll
#define PIT_DIVISOR 14551
#define PIT_PER_SECOND 82

void pit_init();
void pit_interrupt(idt_proc_state_t state);
extern volatile int pit_time;

#endif
