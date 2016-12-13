#include <stdint.h>
#include <stddef.h>

#include "mem/page.h"
#include "mem/kmem.h"
#include "main/printk.h"
#include "main/multiboot.h"
#include "main/panic.h"

static page_t *free_start;
static page_t *used_start;
static page_t static_page;
static int page_id_counter;
static addr_phys_t allocation_pointer;
static page_table_entry_t *cursor;
static mm_entry_t *current_map;
void *virtual_pointer;

void page_init() {
    page_table_t *page_table;
    page_dir_t *page_dir;
    
    allocation_pointer = (addr_phys_t)kmem_map.memory_start - KERNEL_VM_BASE;
    
    for(current_map = &(mb_mem_table[0]);
        (current_map->base + current_map->length) < allocation_pointer;
        current_map ++);
    
    // The cursor in the table to use
    // This is the logical address of the next unallocated space in the kernel address space
    page_dir = kmem_map.vm_start;
    page_table = (page_table_t *)PAGE_TABLE_NOFLAGS(
        page_dir->entries[(uint32_t)kmem_map.vm_end >> PAGE_DIR_SHIFT].table) + KERNEL_VM_BASE;
    cursor = (page_table_entry_t *)((uint32_t)(
            &(page_table->entries[(((uint32_t)kmem_map.vm_end >> PAGE_SHIFT) & 0x3ff)])) + KERNEL_VM_BASE);
    virtual_pointer = kmem_map.vm_end;
}

page_t *page_create(int pid, uint32_t base, uint8_t flags, int count) {
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

page_t *page_alloc(int pid, uint8_t flags, int count) {
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
    allocation_pointer = static_page.mem_base + size;
    
    if((write->mem_base + size) == (current_map->base + current_map->length)) {
#if DEBUG_MEM
        printk("Have to skip to the next memory region.\n");
#endif
        current_map ++;
        for(; current_map->type != 1 && current_map < (mb_mem_table + sizeof(mb_mem_table)); current_map ++);
        if(current_map >= (mb_mem_table + sizeof(mb_mem_table))) {
            panic("Ran out of physical memory!");
        }
        allocation_pointer = current_map->base;
    }
    
#if DEBUG_MEM
    printk("Allocated %d pages.\n", count);
#endif
    
    return write;
}

int page_free(page_t *page) {
    
}

void page_used(page_t *page) {
    page->next = used_start;
    used_start = page;
}

void *page_kinstall(page_t *page, uint8_t page_flags) {
    uint32_t i;
    void *first = NULL;
    for(i = 0; i < page->consecutive; i ++) {
        if((uint32_t)virtual_pointer >= TOTAL_VM_SIZE - PAGE_SIZE) {
            panic("Ran out of kernel virtual address space!");
        }
        
        cursor->block = ((uint32_t)page->mem_base + PAGE_SIZE * i) | page_flags | PAGE_TABLE_PRESENT | PAGE_TABLE_USER;
        if(!first) {
            first = virtual_pointer;
        }
        
        virtual_pointer += PAGE_SIZE;
        
        cursor ++;
    }
    
    kmem_map.memory_end = virtual_pointer;
    
    return first;
}
