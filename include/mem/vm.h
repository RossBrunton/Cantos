#ifndef __H_MEM_VM__
#define __H_MEM_VM__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "main/multiboot.h"
#include "mem/page.h"

typedef struct vm_map_s {
    page_t *physical_dir;
    page_dir_t *logical_dir;
    page_logical_tables_t *logical_tables;
    uint32_t pid;
    uint32_t task_id;
} vm_map_t;

vm_map_t *vm_map_alloc(uint32_t pid, uint32_t task_id, bool kernel);
bool vm_map_new_table(addr_logical_t addr, vm_map_t *map, page_t **page, page_table_t **table, uint8_t page_flags);
void vm_map_insert(addr_logical_t addr, vm_map_t *map, page_t *page, uint8_t page_flags);
void vm_map_free(vm_map_t *map);

void vm_table_switch(addr_phys_t table);
void vm_table_clear();

#endif
