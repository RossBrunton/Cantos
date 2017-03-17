#ifndef _H_INTERRUPTS_LAPIC_
#define _H_INTERRUPTS_LAPIC_

#define LAPIC_ID 0x20
#define LAPIC_VER 0x30
#define LAPIC_TPR 0x80
#define LAPIC_APR 0x90
#define LAPIC_PPR 0xa0
#define LAPIC_EOI 0xb0
#define LAPIC_LOGICAL_DEST 0xd0
#define LAPIC_DESTINATION_FORMAT 0xe0
#define LAPIC_SPURIOUS_INT_VECTOR 0xf0
#define LAPIC_ISR_BASE 0x100
#define LAPIC_TMR_BASE 0x180
#define LAPIC_IRR_BASE 0x200
#define LAPIC_ERROR_STATUS 0x280
#define LAPIC_ICR_A 0x300
#define LAPIC_ICR_B 0x310
#define LAPIC_LVT_TIMER 0x320
#define LAPIC_LVT_THERMAL 0x330
#define LAPIC_LVT_PERFORMANCE 0x340
#define LAPIC_LVT_LINT0 0x350
#define LAPIC_LVT_LINT1 0x350
#define LAPIC_LVT_ERROR 0x360
#define LAPIC_TIMER_INITIAL 0x380
#define LAPIC_TIMER_CURRENT 0x390
#define LAPIC_TIMER_DIVIDE_CONFIGURATION 0x3e0

#define LAPIC_LVT_MASK (1 << 16)
#define LAPIC_TIMER_MODE_PERIODIC (1 << 17)
#define LAPIC_TIMER_MODE_ONE_SHOT (0 << 17)

#define LAPIC_SWITCHES_PER_SECOND 1000

void lapic_init();
void lapic_setup();
void lapic_timer(idt_proc_state_t state);
void lapic_eoi();
void lapic_awaken_others();

#endif
