#include <stdint.h>

#include "main/cpu.hpp"
#include "task/task.hpp"
#include "hw/acpi.h"

extern "C" {
    #include "mem/kmem.h"
    #include "mem/page.h"
}

/** @file main/cpu.c
 *
 * Creates the CPU status for the (as of yet) only CPU.
 *
 * When `cpu_init` is called, the processor is given a stack and set up correctly.
 */

namespace cpu {
    static Status *cpu_status[MAX_CORES]; // Enough space for as many processors as we can get

    extern "C" addr_logical_t *stacks[MAX_CORES];
    addr_logical_t *stacks[MAX_CORES] = {};

    uint32_t id() {
        uint32_t id;

        __asm__ ("\
            mov $1, %%eax\n\
            cpuid\n\
            shr $24, %%ebx\n\
            mov %%ebx, %0"
            : "=r" (id)
            : /* Nothing */
            : "eax", "ebx", "ecx", "edx"
        );

        return id;
    }

    /** */
    Status *info() {
        return cpu_status[id()];
    }

    /** */
    Status *info_of(uint32_t id) {
        return cpu_status[id];
    }

    /** Sets up the (at the moment) only CPU status struct and create a stack for it.
     */
     void init() {
        page_t *page;
        uint32_t i;

        for(i = 0; i < acpi::acpi_proc_count; i ++) {
            page = page_alloc(0, 1);
            cpu_status[i] = new cpu::Status();
            cpu_status[i]->cpu_id = i;
            cpu_status[i]->stack = page_kinstall(page, PAGE_TABLE_RW);
            cpu_status[i]->awoken = false;
            stacks[i] = (addr_logical_t *)(cpu_status[i]->stack);
            cpu_status[i]->thread = NULL;
        }
    }
}
