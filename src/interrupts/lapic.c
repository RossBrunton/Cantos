#include <stdint.h>

#include "interrupts/lapic.h"
#include "mem/page.h"
#include "main/printk.h"
#include "interrupts/exceptions.h"
#include "interrupts/numbers.h"
#include "mem/gdt.h"
#include "io/pit.h"
#include "task/task.h"

static volatile uint32_t *_base;
static volatile uint8_t _stage; // 0 = Waiting for first change, 1 = counting, 2 = done
static volatile uint32_t _deadline;
static volatile uint32_t _callibration_ticks;
static volatile uint32_t _ticks_per_sec;

#define _CAL_INIT 1000
#define _CAL_DIV 3


static uint32_t _read(uint32_t reg) {
    return _base[reg / sizeof(uint32_t)];
}


static void _write(uint32_t reg, uint32_t val) {
    _base[reg / sizeof(uint32_t)] = val;
}


static void _set_timer(uint32_t timer, uint32_t divide) {
    _write(LAPIC_TIMER_INITIAL, timer);
    _write(LAPIC_TIMER_DIVIDE_CONFIGURATION, divide);
    _write(LAPIC_LVT_TIMER, LAPIC_TIMER_MODE_PERIODIC | INT_LAPIC_BASE);
}


void lapic_init() {
    page_t *page;
    
    IDT_ALLOW_INTERRUPT(INT_LAPIC_BASE + INT_LAPIC_TIMER, ltimer);
    
    page = page_create(LAPIC_BASE, PAGE_FLAG_KERNEL, 1);
    _base = page_kinstall(page, PAGE_TABLE_CACHEDISABLE | PAGE_TABLE_RW);
    
    printk("LAPIC ID: %x, Version: %x\n", _read(LAPIC_ID), _read(LAPIC_VER));
    // Set the spurious interrupt vector in order to get interrupts
    _write(LAPIC_SPURIOUS_INT_VECTOR, 0x1ff);
    
    // Set up the timer
    _deadline = pit_time;
    idt_install(INT_LAPIC_BASE, lapic_timer, GDT_SELECTOR(0, 0, 2), IDT_GATE_32_INT);
    _set_timer(_CAL_INIT, _CAL_DIV);
}


void lapic_timer(idt_proc_state_t state) {
    (void)state;
    
    switch(_stage) {
        case 0:
            if(_deadline < pit_time) {
                _stage = 1;
                _deadline = pit_time + PIT_PER_SECOND + 1;
                _callibration_ticks = 1;
            }
            lapic_eoi();
            break;
        
        case 1:
            if(_deadline < pit_time) {
                _stage = 2;
                _ticks_per_sec = _callibration_ticks * _CAL_INIT * _CAL_DIV;
                printk("LAPIC calibration results: %d in one second (setting timer to %d)\n", _ticks_per_sec,
                    _ticks_per_sec / _CAL_DIV / LAPIC_SWITCHES_PER_SECOND);
                //_set_timer(_ticks_per_sec / _CAL_DIV / LAPIC_SWITCHES_PER_SECOND, _CAL_DIV);
                _write(LAPIC_TIMER_INITIAL, _ticks_per_sec / _CAL_DIV / LAPIC_SWITCHES_PER_SECOND);
                _callibration_ticks = 0;
            }else{
                _callibration_ticks ++;
            }
            lapic_eoi();
            break;
        
        case 2:
            lapic_eoi();
            task_timer_yield();
            break;
        
        default:
            lapic_eoi();
            break;
    }
}


void lapic_eoi() {
    _write(LAPIC_EOI, 0xffffffff);
}

#undef _CAL_INIT
#undef _CAL_DIV
