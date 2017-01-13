#include <stdint.h>

#include "main/cpu.h"
#include "mem/kmem.h"
#include "mem/page.h"

/** @file main/cpu.c
 *
 * Creates the CPU status for the (as of yet) only CPU.
 *
 * When `cpu_init` is called, the processor is given a stack and set up correctly.
 */

static cpu_status_t *cpu_status;

/** At the moment, returns a link to a single structure, because multi-CPU support isn't here yet.
 */
cpu_status_t *cpu_info() {
    return cpu_status;
}

/** Sets up the (at the moment) only CPU status struct and create a stack for it.
 */
void cpu_init() {
    page_t *page;
    
    page = page_alloc(0, 0, 1);
    cpu_status = kmalloc(sizeof(cpu_status_t), 0);
    cpu_status[0].cpu_id = 0;
    cpu_status[0].stack = page_kinstall(page, PAGE_TABLE_RW);
    cpu_status[0].thread = NULL;
}
