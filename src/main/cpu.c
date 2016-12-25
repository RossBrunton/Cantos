#include <stdint.h>

#include "main/cpu.h"
#include "mem/kmem.h"
#include "mem/page.h"

static cpu_status_t *cpu_status;

cpu_status_t *cpu_info() {
    return cpu_status;
}

void cpu_init() {
    page_t *page;
    
    page = page_alloc(0, PAGE_FLAG_AUTOKMALLOC, 1);
    cpu_status = kmalloc(sizeof(cpu_status_t));
    cpu_status[0].cpu_id = 0;
    cpu_status[0].stack = page_kinstall(page, PAGE_TABLE_RW);
    cpu_status[0].thread = NULL;
}
