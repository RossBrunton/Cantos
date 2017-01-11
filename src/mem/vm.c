#include <stdint.h>
#include <stddef.h>

#include "mem/vm.h"
#include "mem/page.h"
#include "mem/kmem.h"
#include "main/printk.h"
#include "main/multiboot.h"
#include "main/panic.h"

vm_map_t *vm_map_alloc(uint32_t pid, uint32_t task_id, bool kernel) {
    uint8_t kernel_flag = kernel ? PAGE_FLAG_KERNEL : 0;
    vm_map_t *map;
    size_t i;
    
    map = kmalloc(sizeof(vm_map_t));
    map->physical_dir = page_alloc(pid, kernel_flag, 1);
    map->logical_dir = page_kinstall(map->physical_dir, 0);
    map->logical_tables = kmalloc(sizeof(page_logical_tables_t));
    
    map->pid = pid;
    map->task_id = task_id;
    
    // Load the kernel tables into it
    for(i = 0; i < PAGE_TABLE_LENGTH; i ++) {
        if(i >= PAGE_TABLE_LENGTH - KERNEL_VM_PAGE_TABLES) {
            map->logical_dir->entries[i] = page_dir->entries[i];
        }else{
            map->logical_dir->entries[i].table = 0;
        }
    }
    
    // And zero the logical tables
    for(i = 0; i < (sizeof(map->logical_tables->tables) / sizeof(map->logical_tables->tables[0])); i ++) {
        map->logical_tables->tables[0] = NULL;
        map->logical_tables->pages[0] = NULL;
    }
    
    return map;
}


bool vm_map_new_table(addr_logical_t addr, vm_map_t *map, page_t **page, page_table_t **table, uint8_t page_flags) {
    uint32_t slot = addr >> PAGE_DIR_SHIFT;
    uint8_t kernel_flag = map->pid == 0 ? PAGE_FLAG_KERNEL : 0;
    
    if(map->logical_tables->tables[slot]) {
        return false;
    }else{
        page_t *page_ptr = NULL;
        page_table_t *page_table_ptr = NULL;
        if(!page) {
            page = &page_ptr;
            table = &page_table_ptr;
        }
        if(!*page) {
            *page = page_alloc(map->pid, kernel_flag, 1);
            *table = page_kinstall(*page, 0);
        }
        
        map->logical_tables->pages[slot] = *page;
        map->logical_tables->tables[slot] = *table;
        map->logical_dir->entries[slot].table = (*page)->mem_base | PAGE_TABLE_PRESENT | page_flags;
        return true;
    }
}


void vm_map_insert(addr_logical_t addr, vm_map_t *map, page_t *page, uint8_t page_flags) {
    uint32_t dir_slot = addr >> PAGE_DIR_SHIFT;
    uint32_t page_slot = (addr >> PAGE_TABLE_SHIFT) & PAGE_TABLE_MASK;
    
    if(!map->logical_tables->pages[dir_slot]) {
        panic("Tried to map an address in virtual memory with no page table!");
    }
    
    map->logical_tables->tables[dir_slot]->entries[page_slot].block =
        page->mem_base | page_flags | PAGE_TABLE_PRESENT;
}


void vm_map_free(vm_map_t *map) {
    (void)map;
}


void vm_table_switch(addr_phys_t table) {
    __asm__ volatile ("mov %0, %%cr3" : : "r"(table));
}


void vm_table_clear() {
    __asm__ volatile ("mov %0, %%cr3" : : "r"((addr_phys_t)page_dir - KERNEL_VM_BASE));
}
