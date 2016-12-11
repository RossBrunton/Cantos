#include "main/printk.h"
#include "mem/kmem.h"
#include "mem/page.h"

#include <stdbool.h>

static page_t *kernel_start;
static kmem_free_t *free_list;
static kmem_free_t *free_end;
static kmem_free_t *free_free_structs;

extern char _startofro;
extern char _endofro;
extern char _endofrw;

kmem_map_t kmem_map;

void _print() {
    kmem_free_t *now;
    for(now = free_list; now; now = now->next) {
        printk("[%p@%p %d/%x] ", now->base, now, now->size, now->size);
    }
    printk("\n");
}

void _verify(const char *func) {
    (void)func;
#if DEBUG_MEM
    kmem_free_t *now;
    kmem_free_t *prev = NULL;
    for(now = free_list; now; ((prev = now), (now = now->next))) {
        if(now->next) {
            if(now->next->base < now->base) {
                printk("!!! MEMORY CORRUPTION, List out of order [%s] !!!\n", func);
                _print();
                while(1) {};
            }
            
            if(prev && prev->base + prev->size > now->base) {
                printk("!!! MEMORY CORRUPTION, Overlapping free [%s] !!!\n", func);
                _print();
                while(1) {};
            }
            
            if((size_t)now->base < 0xc01000) {
                printk("!!! MEMORY CORRUPTION, memory less than 0xc01000 [%s] !!!\n", func);
                _print();
                while(1) {};
            }
        }
    }
    
    if(free_end != prev) {
        printk("!!! MEMORY CORRUPTION, End of free array not set correctly [%s] !!!\n", func);
        _print();
        while(1) {};
    }
#endif
}

static void *_memcpy(void *destination, const void *source, size_t num) {
    size_t i;
    for(i = 0; i < num; i ++) {
        ((char *)destination)[i] = ((char *)source)[i];
    }
    return destination;
}


static void _merge_free(kmem_free_t *first) {
    if(first->next && first->base + first->size == first->next->base) {
        kmem_free_t *hold = first->next;
        first->size += hold->size;
        first->next = hold->next;
        hold->next = free_free_structs;
        free_free_structs = hold;
        if(!first->next) {
            free_end = first;
        }
        _verify(__func__);
    }
}


void kmem_init() {
    page_t *initial;
    kmem_header_t header;
    kmem_free_t free_block;
    ptrdiff_t end_pointer = 0;
    void * mem_base;
    
    // Fill in kernel map
    kmem_map.kernel_ro_start = &_startofro;
    kmem_map.kernel_ro_end = &_endofro;
    kmem_map.kernel_rw_start = &_endofro;
    kmem_map.kernel_rw_end = &_endofrw;
    kmem_map.vm_start = &_endofrw;
    kmem_map.vm_end = kmem_map.vm_start;
    kmem_map.vm_end += sizeof(page_dir_entry_t) * PAGE_TABLE_LENGTH;
    kmem_map.vm_end += (sizeof(page_table_entry_t) * PAGE_TABLE_LENGTH) * KERNEL_VM_PAGE_TABLES;
    kmem_map.memory_start = kmem_map.vm_end;
    
    // Set up paging
    page_init();
    
    // Create a page for memory
    initial = page_alloc(0, PAGE_FLAG_KERNEL, 1);
    mem_base = page_kinstall(initial, PAGE_TABLE_RW);
    
    // Memory header for the page header
    header.size = sizeof(page_t);
    _memcpy(mem_base, &header, sizeof(kmem_header_t));
    end_pointer += sizeof(kmem_header_t);
    
    // And the struct
    _memcpy(mem_base+end_pointer, initial, sizeof(page_t));
    kernel_start = mem_base+end_pointer;
    end_pointer += sizeof(page_t);
    
    // And now for the initial free block thing's header
    header.size = sizeof(kmem_free_t);
    _memcpy(mem_base+end_pointer, &header, sizeof(kmem_header_t));
    end_pointer += sizeof(kmem_header_t);
    
    // And its value
    free_block.size = PAGE_SIZE - end_pointer - sizeof(kmem_free_t);
    free_block.base = mem_base + end_pointer + sizeof(kmem_free_t);
    free_block.next = NULL;
    _memcpy(mem_base+end_pointer, &free_block, sizeof(kmem_free_t));
    free_list = mem_base+end_pointer;
    free_end = free_list;
    
    // Create the kernel memory table
    kmem_map.memory_end = free_list->base + free_list->size;
    
    _verify(__func__);
}


void *kmalloc(size_t size) {
    kmem_free_t *free = free_list;
    kmem_free_t *prev = NULL;
    size_t size_needed = 0;
    size_t pages_needed = 0;
    kmem_header_t *hdr = NULL;
    page_t *new_page;
    page_t *page_slot;
    void *installed_loc;
    
    if(size == 0) return NULL;
    
    // Align size to four bytes
    if(size % 4) size += 4 - size % 4;
    
    size_needed = size + sizeof(kmem_header_t);
    
    for(; free; (prev = free), (free = free->next)) {
        if(size_needed <= free->size) {
            // Do it!
            if(size_needed + sizeof(kmem_header_t) >= free->size) {
                // Not a typo: If the remaining space is not enough to store any headers, just use up the whole block
                void *base;
                _verify("kmalloc@before whole block clear");
                
                // Update the free list
                size = free->size - sizeof(kmem_header_t);
                if(!prev) {
                    free_list = free->next;
                }else{
                    prev->next = free->next;
                }
                base = free->base;
                if(free == free_end) {
                    free_end = prev;
                }
                
                // And mix the structure into the free_free_structs
                free->next = free_free_structs;
                free_free_structs = free;
                
                // And then do that malloc thing we were going to do
                hdr = base;
                hdr->size = size;
                _verify("kmalloc@whole block clear");
                return base + sizeof(kmem_header_t);
            }else{
                // Otherwise just shrink it
                _verify("kmalloc@before shrink block");
                void *base = free->base;
                free->size -= size_needed;
                free->base += size_needed;
                hdr = base;
                hdr->size = size;
                _verify("kmalloc@shrink block");
                return base + sizeof(kmem_header_t);
            }
        }
    }
    
    // Allocate a new page
    if(prev && prev->base + prev->size == kmem_map.memory_end) {
        pages_needed = (size_needed + sizeof(page_t) + sizeof(kmem_free_t) - prev->size) / PAGE_SIZE;
    }else{
        pages_needed = (size_needed + sizeof(page_t) + sizeof(kmem_free_t)) / PAGE_SIZE;
    }
    pages_needed += 2;
    
    new_page = page_alloc(0, PAGE_FLAG_KERNEL, pages_needed);
    installed_loc = page_kinstall(new_page, PAGE_TABLE_RW);
    
    if(prev && prev->base + prev->size == installed_loc) {
        // Can just grow the last block because there is free at the end
        prev->size += new_page->consecutive * PAGE_SIZE;
    }else{
        // Well, looks like we have to use the start of the newly allocated block
        // Conveniently there is memory right up to the end of it.
        int space_allocated = new_page->consecutive * PAGE_SIZE - sizeof(kmem_header_t) - sizeof(kmem_free_t);
        hdr = installed_loc;
        hdr->size = sizeof(kmem_free_t);
        free = (kmem_free_t *)(hdr + 1);
        free->size = space_allocated;
        free->base = free + 1;
        free_end->next = free;
        free_end = free;
    }
    
    _verify("kmalloc@end");
    page_slot = kmalloc(sizeof(page_t));
    _memcpy(page_slot, new_page, sizeof(page_t));
    page_used(page_slot);
    return kmalloc(size);
}


static kmem_free_t *_get_struct() {
    kmem_free_t *hold;
    if(free_free_structs) {
        hold = free_free_structs;
        free_free_structs = free_free_structs->next;
        return hold;
    }else{
        hold = kmalloc(sizeof(kmem_free_t));
        _verify(__func__);
        return hold;
    }
}


void kfree(void *ptr) {
    kmem_header_t *hdr = ptr - sizeof(kmem_header_t);
    size_t full_size = hdr->size + sizeof(kmem_header_t);
    
    if(!ptr) {
        // Ignore NULL
        return;
    }
    
    _verify("kfree@start of free");
    if(!free_list) {
        // The list of free entries is empty, make a new one
        kmem_free_t *new_entry = _get_struct();
        new_entry->size = full_size;
        new_entry->base = hdr;
        new_entry->next = NULL;
        free_list = new_entry;
        free_end = new_entry;
    }else{
        // The list exists!
        kmem_free_t *now = free_list;
        kmem_free_t *prev = NULL;
        for(; now && now->base < (void *)hdr; ((prev = now), (now = now->next)));
        kmem_free_t *new_entry = _get_struct();
        _verify("kfree@after get");
        new_entry->size = full_size;
        new_entry->base = hdr;
        new_entry->next = now;
        if(prev) {
            prev->next = new_entry;
        }else{
            free_list = new_entry;
        }
        if(!new_entry->next) {
            if(now) {
                free_end = now;
            }else{
                free_end = new_entry;
            }
        }
        _verify("kfree@before merge");
        _merge_free(new_entry);
        if(prev) _merge_free(prev);
    }
    _verify("kfree@end");
}
