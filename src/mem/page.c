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

void page_init() {
    page_table_t *page_table;
    page_dir_t *page_dir;
    
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
            &(page_table->entries[((kmem_map.vm_end >> PAGE_SHIFT) & 0x3ff)])) + (addr_logical_t)KERNEL_VM_BASE);
    virtual_pointer = kmem_map.vm_end;
}

page_t *page_create(int pid, uint32_t base, uint8_t flags, unsigned int count) {
    page_t *write = &static_page;
    if(flags & PAGE_FLAG_AUTOKMALLOC) {
        write = kmalloc(sizeof(page_t));
    }
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

page_t *page_alloc(int pid, uint8_t flags, unsigned int count) {
    page_t *write = &static_page;
    unsigned int size = count * PAGE_SIZE;
    
    if((allocation_pointer + size) >= (current_map->base + current_map->length)) {
        size = (current_map->base + current_map->length) - allocation_pointer;
    }
    
    if(flags & PAGE_FLAG_AUTOKMALLOC) {
        write = kmalloc(sizeof(page_t));
    }
    write->page_id = page_id_counter ++;
    write->mem_base = allocation_pointer;
    write->flags = PAGE_FLAG_ALLOCATED | flags;
    write->pid = pid;
    write->consecutive = size / PAGE_SIZE;
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
    printk("Allocated %d pages.\n", count);
#endif
    
    if((flags & PAGE_FLAG_AUTOKMALLOC) && write->consecutive < count) {
        write->next = page_alloc(pid, flags, count - write->consecutive);
    }
    
    return write;
}

int page_free(page_t *page) {
    (void)page;
    return 0;
}

void page_used(page_t *page) {
    page->next = used_start;
    used_start = page;
    if(page->next) {
        page_used(page->next);
    }
}

void *page_kinstall(page_t *page, uint8_t page_flags) {
    uint32_t i;
    addr_logical_t first = 0;
    for(i = 0; i < page->consecutive; i ++) {
        if(virtual_pointer >= TOTAL_VM_SIZE - PAGE_SIZE) {
            panic("Ran out of kernel virtual address space!");
        }
        
        cursor->block = (page->mem_base + PAGE_SIZE * i) | page_flags | PAGE_TABLE_PRESENT | PAGE_TABLE_USER;
        if(!first) {
            first = virtual_pointer;
        }
        
        virtual_pointer += PAGE_SIZE;
        
        cursor ++;
    }
    
    kmem_map.memory_end = virtual_pointer;
    
    if(page->next) {
        page_kinstall(page->next, page_flags);
    }
    
    return (void *)first;
}
