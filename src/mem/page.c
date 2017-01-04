#include <stdint.h>
#include <stddef.h>

#include "mem/page.h"
#include "mem/kmem.h"
#include "main/printk.h"
#include "main/multiboot.h"
#include "main/panic.h"

//static page_t *free_start;
static page_t *used_start;
static page_t static_page;
static int page_id_counter;
static addr_phys_t allocation_pointer;
static page_table_entry_t *cursor;
static mm_entry_t *current_map;
static addr_logical_t virtual_pointer;
static page_dir_t *page_dir;
static page_t *page_free_head;

static void *_memcpy(void *destination, const void *source, size_t num) {
    size_t i;
    for(i = 0; i < num; i ++) {
        ((char *)destination)[i] = ((char *)source)[i];
    }
    return destination;
}


static void _verify(const char *func) {
    (void)func;
#if DEBUG_MEM
    page_t *now;
    page_t *prev = NULL;
    for(now = page_free_head; now; ((prev = now), (now = now->next))) {
        if(now->next) {
            if(now->next->mem_base < now->mem_base) {
                panic("Free page list corruption, List out of order [%s]", func);
            }
            
            if(prev && prev->mem_base + (prev->consecutive * PAGE_SIZE) > now->mem_base) {
                panic("Free page list corruption, Overlapping pages [%s]", func);
            }
        }
    }
#endif
}


static void _merge_free(page_t *first) {
    if(first->next && first->mem_base + (first->consecutive * PAGE_SIZE) == first->next->mem_base) {
        page_t *hold = first->next;
        first->consecutive += hold->consecutive;
        first->next = hold->next;
        kfree(hold);
        _verify(__func__);
    }
}


void page_init() {
    page_table_t *page_table;
    
    allocation_pointer = (addr_phys_t)kmem_map.memory_start - KERNEL_VM_BASE;
    
    for(current_map = &(mb_mem_table[0]);
        (current_map->base + current_map->length) < allocation_pointer;
        current_map ++);
    
    // The cursor in the table to use
    // This is the logical address of the next unallocated space in the kernel address space
    page_dir = (page_dir_t *)kmem_map.vm_start;
    page_table = (page_table_t *)PAGE_TABLE_NOFLAGS(
        page_dir->entries[kmem_map.vm_end >> PAGE_DIR_SHIFT].table) + KERNEL_VM_BASE;
    cursor = (page_table_entry_t *)((addr_logical_t)(
        &(page_table->entries[((kmem_map.vm_end >> PAGE_TABLE_SHIFT) & PAGE_TABLE_MASK)]))
            + (addr_logical_t)KERNEL_VM_BASE);
    virtual_pointer = kmem_map.vm_end;
}


page_t *page_create(int pid, uint32_t base, uint8_t flags, unsigned int count) {
    page_t *write = &static_page;
    write = kmalloc(sizeof(page_t));
    write->page_id = page_id_counter ++;
    write->mem_base = base;
    write->flags = PAGE_FLAG_ALLOCATED | flags;
    write->pid = pid;
    write->consecutive = count;
#if DEBUG_MEM
    printk("Allocated %d pages.\n", count);
#endif
    
    return write;
}


page_t *page_alloc_nokmalloc(int pid, uint8_t flags, unsigned int count) {
    page_t *write = &static_page;
    unsigned int size = count * PAGE_SIZE;
    
    if((allocation_pointer + size) >= (current_map->base + current_map->length)) {
        size = (current_map->base + current_map->length) - allocation_pointer;
    }
    
    write->page_id = page_id_counter ++;
    write->mem_base = allocation_pointer;
    write->flags = PAGE_FLAG_ALLOCATED | flags;
    write->pid = pid;
    write->consecutive = size / PAGE_SIZE;
    write->next = NULL;
    allocation_pointer = write->mem_base + size;
    
    if((write->mem_base + size) == (current_map->base + current_map->length)) {
#if DEBUG_MEM
        printk("Have to skip to the next memory region.\n");
#endif
        current_map ++;
        for(; current_map->type != 1 && current_map < (mb_mem_table + sizeof(mb_mem_table)); current_map ++);
        if(current_map >= (mb_mem_table + sizeof(mb_mem_table))) {
            panic("Ran out of physical memory!");
        }
        if(current_map->base > UINT32_MAX) {
            panic("Ran out of addressable physical memory!");
        }
        allocation_pointer = current_map->base;
    }
    
#if DEBUG_MEM
    printk("Allocated %d pages at %p.\n", count, write->mem_base);
#endif
    
    return write;
}


page_t *page_alloc(int pid, uint8_t flags, unsigned int count) {
    page_t *new;
    
    // Collect all the pages in the free list
    if(page_free_head) {
        if(page_free_head->consecutive > count) {
            new = kmalloc(sizeof(page_t));
            new->page_id = page_id_counter ++;
            new->mem_base = page_free_head->mem_base;
            page_free_head->mem_base += (page_free_head->consecutive - count) * PAGE_SIZE;
            page_free_head->consecutive -= count;
        }else{
            new = page_free_head;
            page_free_head = new->next;
        }
        new->flags = flags;
        new->pid = pid;
        new->next = NULL;
        _verify(__func__);
    }else{
        new = kmalloc(sizeof(page_t));
        page_alloc_nokmalloc(pid, flags, count);
        _memcpy(new, &static_page, sizeof(page_t));
        
        if(new->consecutive < count) {
            new->next = page_alloc(pid, flags, count - new->consecutive);
        }
    }
    
    return new;
}


void page_free(page_t *page) {
    page_t *now;
    page_t *prev = NULL;
    
    if(!page) {
        return;
    }
    
    if(page->next) {
        page_free(page->next);
    }
    
    for(now = page_free_head; now && now->mem_base < page->mem_base; ((prev = now), (now = now->next)));
    if(prev) {
        prev->next = page;
    }else{
        page_free_head = page;
    }
    page->next = now;
    
    // Try to flatten the free entries
    if(prev) _merge_free(prev);
    _merge_free(page);
}


void page_used(page_t *page) {
    page_t *old_next = page->next;
    page->next = used_start;
    used_start = page;
    if(old_next) {
        page_used(old_next);
    }
}


void *page_kinstall(page_t *page, uint8_t page_flags) {
    uint32_t i;
    addr_logical_t first = 0;
    addr_phys_t base = 0;
    for(i = 0; i < page->consecutive; i ++) {
        if(virtual_pointer >= TOTAL_VM_SIZE - PAGE_SIZE) {
            panic("Ran out of kernel virtual address space!");
        }
        
        cursor->block = (page->mem_base + PAGE_SIZE * i) | page_flags | PAGE_TABLE_PRESENT;
        if(!first) {
            first = virtual_pointer;
            base = page->mem_base;
        }
        
        virtual_pointer += PAGE_SIZE;
        
        cursor ++;
    }
    
    kmem_map.memory_end = virtual_pointer;
    
    if(page->next) {
        page_kinstall(page->next, page_flags);
    }
    
#if DEBUG_MEM
    printk("KInstalled %d pages at %p onto %p.\n", page->consecutive, base, first);
#endif
    
    return (void *)first;
}


page_vm_map_t *page_alloc_vm_map(uint32_t pid, uint32_t task_id, bool kernel) {
    uint8_t kernel_flag = kernel ? PAGE_FLAG_KERNEL : 0;
    page_vm_map_t *map;
    size_t i;
    
    map = kmalloc(sizeof(page_vm_map_t));
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


bool page_vm_map_new_table
    (addr_logical_t addr, page_vm_map_t *map, page_t **page, page_table_t **table, uint8_t page_flags) {
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


void page_vm_map_insert(addr_logical_t addr, page_vm_map_t *map, page_t *page, uint8_t page_flags) {
    uint32_t dir_slot = addr >> PAGE_DIR_SHIFT;
    uint32_t page_slot = (addr >> PAGE_TABLE_SHIFT) & PAGE_TABLE_MASK;
    
    if(!map->logical_tables->pages[dir_slot]) {
        panic("Tried to map an address in virtual memory with no page table!");
    }
    
    map->logical_tables->tables[dir_slot]->entries[page_slot].block =
        page->mem_base | page_flags | PAGE_TABLE_PRESENT;
}


void page_free_vm_map(page_vm_map_t *map) {
    (void)map;
}


void page_table_switch(addr_phys_t table) {
    __asm__ volatile ("mov %0, %%cr3" : : "r"(table));
}


void page_table_clear() {
    __asm__ volatile ("mov %0, %%cr3" : : "r"((addr_phys_t)page_dir - KERNEL_VM_BASE));
}
