#include <stdint.h>

#include "main/cpu.hpp"
#include "task/task.hpp"

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
    static Status *cpu_status;

    /** At the moment, returns a link to a single structure, because multi-CPU support isn't here yet.
     */
    Status *info() {
        return cpu_status;
    }

    /** Sets up the (at the moment) only CPU status struct and create a stack for it.
     */
     void init() {
        page_t *page;

        page = page_alloc(0, 1);
        cpu_status = new cpu::Status();
        cpu_status[0].cpu_id = 0;
        cpu_status[0].stack = page_kinstall(page, PAGE_TABLE_RW);
        cpu_status[0].thread = NULL;
    }
}
