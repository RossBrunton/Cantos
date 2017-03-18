#include <stdint.h>

#include "main/cpu.hpp"
#include "hw/pit.hpp"
#include "int/idt.hpp"
#include "int/lapic.hpp"
#include "main/printk.hpp"

extern "C" {
    #include "mem/page.h"
    #include "int/numbers.h"
    #include "mem/gdt.h"
    #include "task/task.h"
    #include "main/lomain.h"
    #include "mem/gdt.h"
    #include "hw/acpi.h"
    #include "hw/utils.h"
}

namespace lapic {
    static volatile uint32_t *_base;
    static volatile uint8_t _stage; // 0 = Waiting for first change, 1 = counting, 2 = done
    static volatile uint32_t _deadline;
    static volatile uint32_t _callibration_ticks;
    static volatile uint32_t _ticks_per_sec;

    extern "C" volatile char _startofap;
    extern "C" volatile char _endofap;
    extern "C" volatile uint32_t low_ap_page_table;

    static volatile bool _calibrated[MAX_CORES];

    const uint32_t _CAL_INIT = 1000;
    const uint32_t _CAL_DIV = 3;
    const uint32_t _JUMP_BASE = 0x1000;

    static uint32_t _read(uint32_t reg) {
        return _base[reg / sizeof(uint32_t)];
    }


    static void _write(uint32_t reg, uint32_t val) {
        _base[reg / sizeof(uint32_t)] = val;
    }


    static void _set_timer(uint32_t timer, uint32_t divide) {
        _write(TIMER_INITIAL, timer);
        _write(TIMER_DIVIDE_CONFIGURATION, divide);
        _write(LVT_TIMER, TIMER_MODE_PERIODIC | INT_LAPIC_BASE);
    }


    static void _ipi(uint32_t vector, uint8_t dest, uint8_t init_deassert, uint32_t target) {
        uint32_t val = 0;
        val |= vector & 0xff;
        val |= dest << 8;
        val |= init_deassert << 14;
        
        _write(ICR_B, target << 24);
        _write(ICR_A, val);
    }

    IDT_TELL_INTERRUPT(ltimer);
    void init() {
        page_t *page;
        
        IDT_ALLOW_INTERRUPT(INT_LAPIC_BASE + INT_LAPIC_TIMER, ltimer);
        
        page = page_create(acpi::acpi_lapic_base, PAGE_FLAG_KERNEL, 1);
        _base = (uint32_t *)page_kinstall(page, PAGE_TABLE_CACHEDISABLE | PAGE_TABLE_RW);
        
        printk("LAPIC ID: %x, Version: %x\n", _read(ID), _read(VER));
        // Set the spurious interrupt vector in order to get interrupts
        _write(SPURIOUS_INT_VECTOR, 0x1ff);
        
        // Set up the timer
        _deadline = pit::time;
        idt::install(INT_LAPIC_BASE, timer, GDT_SELECTOR(0, 0, 2), idt::GATE_32_INT);
        _set_timer(_CAL_INIT, _CAL_DIV);
    }


    void setup() {
        _write(SPURIOUS_INT_VECTOR, 0x1ff);
        eoi();
        
        // Set up the timer
        _deadline = pit::time;
        _set_timer(_CAL_INIT, _CAL_DIV);
    }


    void timer(idt_proc_state_t state) {
        (void)state;
        
        if(cpu::id() == 0) {
            switch(_stage) {
                case 0:
                    if(_deadline < pit::time) {
                        _stage = 1;
                        _deadline = pit::time + pit::PER_SECOND + 1;
                        _callibration_ticks = 1;
                    }
                    eoi();
                    break;
                
                case 1:
                    if(_deadline < pit::time) {
                        _stage = 2;
                        _ticks_per_sec = _callibration_ticks * _CAL_INIT * _CAL_DIV;
                        printk("LAPIC calibration results: %d in one second (setting timer to %d)\n", _ticks_per_sec,
                            _ticks_per_sec / _CAL_DIV / SWITCHES_PER_SECOND);
                        //_set_timer(_ticks_per_sec / _CAL_DIV / SWITCHES_PER_SECOND, _CAL_DIV);
                        _write(TIMER_INITIAL, _ticks_per_sec / _CAL_DIV / SWITCHES_PER_SECOND);
                        _callibration_ticks = 0;
                    }else{
                        _callibration_ticks ++;
                    }
                    eoi();
                    break;
                
                case 2:
                    eoi();
                    task_timer_yield();
                    break;
                
                default:
                    eoi();
                    break;
            }
        }else{
            uint32_t id = cpu::id();
            
            if(_calibrated[id]) {
                eoi();
                task_timer_yield();
            }else{
                if(_ticks_per_sec) {
                    printk("AP: %d in one second (setting timer to %d)\n", _ticks_per_sec,
                            _ticks_per_sec / _CAL_DIV / SWITCHES_PER_SECOND);
                    //_set_timer(_ticks_per_sec / _CAL_DIV / SWITCHES_PER_SECOND, _CAL_DIV);
                    _write(TIMER_INITIAL, _ticks_per_sec / _CAL_DIV / SWITCHES_PER_SECOND);
                    _calibrated[id] = true;
                }else{
                    // Do nothing
                }
                eoi();
            }
        }
    }


    void awaken_others() {
        uint32_t i;
        
        // Lets see if this works...
        for(i = 0; i < (uint32_t)&_endofap - (uint32_t)&_startofap; i ++) {
            //printk("0x%x: 0x%x\n", ((uint8_t *)_JUMP_BASE + i), *(&_startofap + i));
            *((uint8_t *)_JUMP_BASE + i) = *(&_startofap + i);
        }
        
        low_ap_page_table = kmem_map.vm_start - KERNEL_VM_BASE;
        
        // Loop through and wake them all up (except number 0)
        for(i = 1; i < acpi::acpi_proc_count; i ++) {
            uint32_t id = acpi::acpi_procs[i].apic_id;
            
            _ipi(0, 5, 1, id);
            io_wait();
            _ipi((uint32_t)_JUMP_BASE / PAGE_SIZE, 6, 1, id);
        }
        
        while(true) {};
    }


    void eoi() {
        _write(EOI, 0xffffffff);
    }
}
