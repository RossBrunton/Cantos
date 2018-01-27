#include <stdint.h>

#include "main/cpu.hpp"
#include "hw/pit.hpp"
#include "int/idt.hpp"
#include "int/lapic.hpp"
#include "main/printk.hpp"
#include "mem/gdt.hpp"
#include "mem/kmem.hpp"
#include "mem/page.hpp"
#include "task/task.hpp"
#include "int/numbers.h"
#include "hw/acpi.h"
#include "hw/utils.h"
#include "structures/mutex.hpp"
#include "main/asm_utils.hpp"

namespace lapic {
    static volatile uint32_t *_base;
    static volatile uint8_t _stage; // 0 = Waiting for first change, 1 = counting, 2 = done
    static volatile uint32_t _deadline;
    static volatile uint32_t _callibration_ticks;
    static volatile uint32_t _ticks_per_sec;

    static mutex::Mutex _command_mutex;

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
    IDT_TELL_INTERRUPT(lcommand);
    void init() {
        page::Page *page;

        IDT_ALLOW_INTERRUPT(INT_LAPIC_BASE + INT_LAPIC_TIMER, ltimer);
        IDT_ALLOW_INTERRUPT(INT_LAPIC_BASE + INT_LAPIC_COMMAND, lcommand);

        page = page::create(acpi::acpi_lapic_base, page::FLAG_KERNEL, 1);
        _base = (uint32_t *)page::kinstall(page, page::PAGE_TABLE_CACHEDISABLE | page::PAGE_TABLE_RW);

        printk("LAPIC ID: %x, Version: %x\n", _read(ID), _read(VER));
        // Set the spurious interrupt vector in order to get interrupts
        _write(SPURIOUS_INT_VECTOR, 0x1ff);

        // Set up the timer
        _deadline = pit::time;
        idt::install(INT_LAPIC_BASE, timer, GDT_SELECTOR(0, 0, 2), idt::GATE_32_INT);
        _set_timer(_CAL_INIT, _CAL_DIV);

        // And the command handler
        idt::install(INT_LAPIC_BASE + INT_LAPIC_COMMAND, handle_command, GDT_SELECTOR(0, 0, 2), idt::GATE_32_INT);
    }


    void setup() {
        _write(SPURIOUS_INT_VECTOR, 0x1ff);
        eoi();

        // Set up the timer
        _set_timer(_CAL_INIT, _CAL_DIV);
    }


    void timer(idt_proc_state_t state) {
        (void)state;

        uint32_t id = cpu::id();
        if(id == 0) {
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
                        _write(TIMER_INITIAL, _ticks_per_sec / _CAL_DIV / SWITCHES_PER_SECOND);
                        _callibration_ticks = 0;
                    }else{
                        _callibration_ticks ++;
                    }
                    eoi();
                    break;

                case 2:
                    eoi();
                    task::task_timer_yield();
                    break;

                default:
                    eoi();
                    break;
            }
        }else{
            if(_calibrated[id]) {
                eoi();
                task::task_timer_yield();
            }else{
                if(_ticks_per_sec) {
                    _set_timer(_ticks_per_sec / _CAL_DIV / SWITCHES_PER_SECOND, _CAL_DIV);
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

        low_ap_page_table = kmem::map.vm_start - KERNEL_VM_BASE;

        // Loop through and wake them all up (except number 0)
        for(i = 1; i < acpi::acpi_proc_count && i < MAX_CORES; i ++) {
            uint32_t id = acpi::acpi_procs[i].apic_id;

            _ipi(0, 5, 1, id);
            io_wait();
            _ipi((uint32_t)_JUMP_BASE / PAGE_SIZE, 6, 1, id);

            while(!cpu::info_of(id).awoken);
        }
    }


    void eoi() {
        _write(EOI, 0xffffffff);
    }


    void ipi(uint8_t vector, uint32_t proc) {
        // TODO: LAPIC CPU ids probably don't equal cantos CPU ids
        _ipi(vector, 0, 0, proc);
    }

    void ipi_all(uint8_t vector) {
        for(uint32_t i = 0; i < acpi::acpi_proc_count && i < MAX_CORES; i ++) {
            ipi(vector, i);
        }
    }

    void send_command(command_t command, uint32_t argument, uint32_t proc) {
        _command_mutex.lock();
        cpu::Status &status = cpu::info_of(proc);
        status.command_finished = 0;
        status.command = command;
        status.command_arg = argument;
        ipi(INT_LAPIC_BASE + INT_LAPIC_COMMAND, proc);
        while(!status.command_finished) {};
        _command_mutex.unlock();
    }

    void send_command_all(command_t command, uint32_t argument) {
        // TODO: This better
        uint32_t eflags = push_cli();
        uint32_t id = cpu::id();
        for(uint32_t i = 0; i < acpi::acpi_proc_count; i ++) {
            if(i != id) {
                send_command(command, argument, i);
            }
        }

        pop_flags(eflags);
    }

    void handle_command(idt_proc_state_t state) {
        cpu::Status &status = cpu::info();

        switch(status.command) {
            case CMD_INVLPG:
                asm volatile ("invlpg (%0)" : : "r"(status.command_arg));
                break;

            default:
                panic("Unknown IPI command 0x%x", status.command);
        }

        status.command_finished = true;
        eoi();
    }
}
