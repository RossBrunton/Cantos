#include <stdint.h>
#include <stddef.h>

#include "mem/page.h"
#include "main/printk.h"
#include "main/multiboot.h"

extern char _endofelf;
static page_t free_list;
static page_t *free_list_end;
static int page_id_counter;
static mm_entry_t mem_table[10];

static void *_memcpy(void * destination, const void * source, size_t num) {
    size_t i;
    for(i = 0; i < num; i ++) {
        ((char *)destination)[i] = ((char *)source)[i];
    }
    return destination;
}

void page_init(multiboot_info_t *mbi) {
    mm_entry_t *entry;
    signed int i;
    
    // Copy the memory table into our little safe place
    entry = (void*)mbi->mmap_addr;
    for(i = 0; (uint32_t)((void *)entry - mbi->mmap_addr) < mbi->mmap_length; i ++) {
        _memcpy(&(mem_table[i]), entry, sizeof(mm_entry_t));
        entry = (mm_entry_t *)(((void *)entry) + entry->size + 4);
    }
    
    // Get the first page
    free_list.page_id = page_id_counter ++;
    free_list.mem_base_and_free = (size_t)(&_endofelf + PAGE_SIZE) & ~PAGE_FREE_MASK;
    free_list.flags = PAGE_FLAG_ALLOCATED | PAGE_FLAG_KERNEL;
    
    printk("First page allocated at %p\n", free_list.mem_base_and_free);
}

page_t *page_alloc(int pid, uint8_t flags) {
    
}

int page_free(page_t *page) {
    page->mem_base_and_free &= ~PAGE_FREE_MASK;
    page->flags = 0;
    page->pid = -1;
    page->next = NULL;
    
    page->prev = free_list_end;
    free_list_end->next = page;
    free_list_end = page;
}
