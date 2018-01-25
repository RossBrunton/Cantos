#include <stdint.h>

#include "main/cpu.hpp"
#include "task/task.hpp"
#include "hw/acpi.h"
#include "mem/page.hpp"
#include "main/panic.hpp"
#include "main/asm_utils.hpp"
#include "structures/unique_ptr.hpp"
#include "structures/shared_ptr.hpp"

/** @file main/cpu.c
 *
 * Creates the CPU status for the (as of yet) only CPU.
 *
 * When `cpu_init` is called, the processor is given a stack and set up correctly.
 */

namespace cpu {
    static unique_ptr<Status> cpu_status[MAX_CORES]; // Enough space for as many processors as we can get

    extern "C" addr_logical_t *stacks[MAX_CORES];
    addr_logical_t *stacks[MAX_CORES] = {};

    uint32_t id() {
        CHECK_IF_CLR;
        uint32_t volatile id;

        __asm__ volatile ("\
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

    Status& info() {
        return info_of(id());
    }

    Status& info_of(uint32_t id) {
        if(!cpu_status[id]) {
            panic("Tried to get the status of an invalid or un-inited cpu (%d)", id);
        }
        return *cpu_status[id];
    }

     void init() {
        page::Page *page;
        uint32_t i;

        for(i = 0; i < acpi::acpi_proc_count; i ++) {
            page = page::alloc(0, 1);
            cpu_status[i] = make_unique<Status>();
            cpu_status[i]->cpu_id = i;
            cpu_status[i]->stack = page::kinstall(page, page::PAGE_TABLE_RW);
            cpu_status[i]->awoken = false;
            stacks[i] = (addr_logical_t *)(cpu_status[i]->stack);
            cpu_status[i]->thread = NULL;
        }
    }

    shared_ptr<task::Thread>current_thread() {
        shared_ptr<task::Thread> t;
        uint32_t eflags = push_cli();
        t = cpu_status[id()]->thread;
        pop_flags(eflags);
        return t;
    }

    shared_ptr<task::Thread>current_thread_noint() {
        CHECK_IF_CLR;
        return cpu_status[id()]->thread;
    }
}
