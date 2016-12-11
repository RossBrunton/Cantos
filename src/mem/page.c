#include <stdint.h>
#include <stddef.h>

#include "mem/page.h"
#include "mem/kmem.h"
#include "main/printk.h"
#include "main/multiboot.h"

static page_t *free_start;
static page_t *used_start;
static page_t static_page;
static int page_id_counter;
static addr_phys_t allocation_pointer;
static page_table_entry_t *cursor;
void *virtual_pointer;

void page_init() {
    page_table_t *page_table;
    page_dir_t *page_dir;
    
    allocation_pointer = (addr_phys_t)kmem_map.memory_start - KERNEL_VM_BASE;
    
    // The cursor in the table to use
    // This is the logical address of the next unallocated space in the kernel address space
    page_dir = kmem_map.vm_start;
    page_table = (page_table_t *)PAGE_TABLE_NOFLAGS(
        page_dir->entries[(uint32_t)kmem_map.vm_end >> PAGE_DIR_SHIFT].table) + KERNEL_VM_BASE;
    cursor = &(page_table->entries[(((uint32_t)kmem_map.vm_end >> PAGE_SHIFT) & 0x3ff)]);
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
    if(flags & PAGE_FLAG_AUTOKMALLOC) {
        write = kmalloc(sizeof(page_t));
    }
    write->page_id = page_id_counter ++;
    write->mem_base = allocation_pointer;
    write->flags = PAGE_FLAG_ALLOCATED | flags;
    write->pid = pid;
    write->consecutive = count;
    allocation_pointer = static_page.mem_base + PAGE_SIZE;
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
