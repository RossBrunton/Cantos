#include "main/printk.h"
#include "mem/kmem.h"
#include "mem/page.h"
#include "main/panic.h"

#include <stdbool.h>

#define _MINIMUM_PAGES 2

static page_t *kernel_start;
static kmem_free_t *free_list;
static kmem_free_t *free_end;
static kmem_free_t *free_free_structs;
static volatile uint32_t memory_total;
static volatile uint32_t memory_used;

extern char _startofro;
extern char _endofro;
extern char _endofrw;

kmem_map_t kmem_map;

void _print() {
    kmem_free_t *now;
    int i;
    for((now = free_list), (i = 0); now && i < 1000; (now = now->next), (i ++)) {
        printk("[%p@%p %d/%x] ", now->base, now, now->size, now->size);
    }
    if(i >= 999) {
        printk("[...]");
    }
    printk("\n");
}

void _print_frees() {
    kmem_free_t *now;
    int i;
    for((now = free_free_structs), (i = 0); now && i < 1000; (now = now->next), (i ++)) {
        printk("{%p@%p %d/%x} ", now->base, now, now->size, now->size);
    }
    if(i >= 999) {
        printk("{...}");
    }
    printk("\n");
}

static void _verify(const char *func) {
    (void)func;
#if DEBUG_MEM
    kmem_free_t *now;
    kmem_free_t *prev = NULL;
    kmem_free_t *frees = NULL;
    for(now = free_list; now; ((prev = now), (now = now->next))) {
        if(now->next) {
            if(now->next->base < now->base) {
                _print();
                panic("Memory corruption, list out of order [%s]", func);
            }
            
            if(prev && prev->base == now->base) {
                _print();
                panic("Memory corruption, same start address [%s] (prev: %p, now: %p)", func, prev, now);
            }
            
            if(prev && prev->base + prev->size > now->base) {
                _print();
                panic("Memory corruption, overlapping free [%s] (prev: %p, now: %p)", func, prev, now);
            }
            
            if((size_t)now->base < 0xc01000) {
                _print();
                panic("Memory corruption, memory less than 0xc01000 [%s]\n", func);
            }
            
            if(now->next == now) {
                _print();
                panic("Memory corruption, free linked to itself [%s]\n", func);
            }
        }
        
        for(frees = free_free_structs; frees; frees = frees->next) {
            if(frees == now) {
                _print();
                panic("Memory corruption, entry is in free structs list [%s]\n", func);
            }
        }
    }
    
    if(free_end != prev) {
        _print();
        panic("Memory corruption, end of free array not set correctly [%s]", func);
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
    addr_logical_t mem_base;
    page_dir_t *dir;
    
    // Fill in kernel map
    kmem_map.kernel_ro_start = (addr_logical_t)&_startofro;
    kmem_map.kernel_ro_end = (addr_logical_t)&_endofro;
    kmem_map.kernel_rw_start = (addr_logical_t)&_endofro;
    kmem_map.kernel_rw_end = (addr_logical_t)&_endofrw;
    kmem_map.vm_start = (addr_logical_t)&_endofrw;
    kmem_map.vm_end = kmem_map.vm_start;
    kmem_map.vm_end += sizeof(page_dir_entry_t) * PAGE_TABLE_LENGTH;
    kmem_map.vm_end += (sizeof(page_table_entry_t) * PAGE_TABLE_LENGTH) * KERNEL_VM_PAGE_TABLES;
    kmem_map.memory_start = kmem_map.vm_end;
    
    // Set up paging
    page_init();
    
    // Create a page for memory
    initial = page_alloc_nokmalloc(PAGE_FLAG_KERNEL, 1);
    mem_base = (addr_logical_t)page_kinstall_append(initial, PAGE_TABLE_RW);
    
    // Memory header for the page header
    header.size = sizeof(page_t);
    _memcpy((void *)mem_base, &header, sizeof(kmem_header_t));
    end_pointer += sizeof(kmem_header_t);
    
    // And the struct
    _memcpy((void *)(mem_base+end_pointer), initial, sizeof(page_t));
    kernel_start = (page_t *)(mem_base + end_pointer);
    end_pointer += sizeof(page_t);
    
    // And now for the initial free block thing's header
    header.size = sizeof(kmem_free_t);
    _memcpy((void *)(mem_base+end_pointer), &header, sizeof(kmem_header_t));
    end_pointer += sizeof(kmem_header_t);
    
    // And its value
    free_block.size = PAGE_SIZE - end_pointer - sizeof(kmem_free_t);
    free_block.base = mem_base + end_pointer + sizeof(kmem_free_t);
    free_block.next = NULL;
    _memcpy((void *)(mem_base+end_pointer), &free_block, sizeof(kmem_free_t));
    free_list = (kmem_free_t *)(mem_base+end_pointer);
    free_end = free_list;
    end_pointer += sizeof(kmem_free_t);
    
    // Create the kernel memory table
    kmem_map.memory_end = free_list->base + free_list->size;
    
    // Clear the first 1MiB
    dir = (page_dir_t *)kmem_map.vm_start;
    dir->entries[0].table = 0x0;
    
    // Set the initial memory values
    memory_total = PAGE_SIZE;
    memory_used = end_pointer;
    
    _verify(__func__);
}


void *kmalloc(size_t size, uint8_t flags) {
    kmem_free_t *free = free_list;
    kmem_free_t *prev = NULL;
    size_t size_needed = 0;
    size_t pages_needed = 0;
    kmem_header_t *hdr = NULL;
    page_t *new_page;
    addr_logical_t installed_loc;
#if DEBUG_VMEM
    printk("Allocating %d bytes.\n", size);
#endif
    
    if(size == 0) return NULL;
    
    // Align size to four bytes
    if(size % 4) size += 4 - size % 4;
    
    // And ensure the size is at least the size of a page, so some nasty person doesn't pollute the memory with
    //  2 word entries followed by a 2 word gap, meaning that more pages can't be allocated despite there seeming to be
    //  room
    if(size < sizeof(page_t)) size = sizeof(page_t);
    
    size_needed = size + sizeof(kmem_header_t);
    
    if((((uint64_t)memory_used + (uint64_t)size_needed > memory_total - (_MINIMUM_PAGES * PAGE_SIZE))
    || memory_total < (_MINIMUM_PAGES * PAGE_SIZE))
    && !(flags & KMALLOC_RESERVED)) {
#if DEBUG_MEM
        printk("Wanted to kmalloc more than we can safely store.\n");
#endif
    }else{
        for(; free; (prev = free), (free = free->next)) {
            if(size_needed <= free->size) {
                // Do it!
                if(size_needed + sizeof(kmem_header_t) >= free->size) {
                    // If the remaining space is not enough to store any more headers, just use up the whole block
                    addr_logical_t base;
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
                    hdr = (kmem_header_t *)base;
                    hdr->size = size;
                    _verify("kmalloc@whole block clear");
                    memory_used += size + sizeof(kmem_header_t);
                    return (void *)(base + sizeof(kmem_header_t));
                }else{
                    // Otherwise just shrink it
                    _verify("kmalloc@before shrink block");
                    addr_logical_t base = free->base;
                    free->size -= size_needed;
                    free->base += size_needed;
                    hdr = (kmem_header_t *)base;
                    hdr->size = size;
                    _verify("kmalloc@shrink block");
                    memory_used += size_needed;
                    return (void *)(base + sizeof(kmem_header_t));
                }
            }
        }
    }
    
    // Allocate a new page
    if(prev && prev->base + prev->size == kmem_map.memory_end) {
        pages_needed = (size_needed + sizeof(page_t) + sizeof(kmem_free_t) - prev->size) / PAGE_SIZE;
    }else{
        pages_needed = (size_needed + sizeof(page_t) + sizeof(kmem_free_t)) / PAGE_SIZE;
    }
    pages_needed += _MINIMUM_PAGES;
    
#if DEBUG_MEM
    printk("Extending memory by %d pages (%x/%x).\n", pages_needed, memory_used, memory_total);
#endif
    
    // This calls kmalloc, be careful!
    new_page = page_alloc(PAGE_FLAG_KERNEL | PAGE_FLAG_RESERVED, pages_needed);
    installed_loc = (addr_logical_t)page_kinstall_append(new_page, PAGE_TABLE_RW);
    memory_total += pages_needed * PAGE_SIZE;
    
    if(prev && prev->base + prev->size == installed_loc) {
        // Can just grow the last block because there is free at the end
        prev->size += new_page->consecutive * PAGE_SIZE;
    }else{
        // Well, looks like we have to use the start of the newly allocated block
        // Conveniently there is memory right up to the end of it.
        int space_allocated = new_page->consecutive * PAGE_SIZE - sizeof(kmem_header_t) - sizeof(kmem_free_t);
        hdr = (kmem_header_t *)installed_loc;
        hdr->size = sizeof(kmem_free_t);
        free = (kmem_free_t *)(hdr + 1);
        free->size = space_allocated;
        free->base = (addr_logical_t)(free + 1);
        free->next = NULL;
        if(free_end) {
            free_end->next = free;
            free_end = free;
        }else{
            free_list = free;
        }
        free_end = free;
    }
    
    _verify("kmalloc@end");
    page_used(new_page);
    return kmalloc(size, flags);
}


static kmem_free_t *_get_struct() {
    kmem_free_t *hold;
    if(free_free_structs) {
        hold = free_free_structs;
        free_free_structs = free_free_structs->next;
        _verify(__func__);
        return hold;
    }else{
        hold = kmalloc(sizeof(kmem_free_t), KMALLOC_RESERVED);
        _verify(__func__);
        return hold;
    }
}


void kfree(void *ptr) {
    kmem_header_t *hdr = (kmem_header_t *)ptr - 1;
    size_t full_size = hdr->size + sizeof(kmem_header_t);
    kmem_free_t *new_entry = _get_struct();
    
#if DEBUG_VMEM
    printk("Freeing %p (%d bytes).\n", ptr, hdr->size);
#endif
    
    if(!ptr) {
        // Ignore NULL
        return;
    }
    
    _verify("kfree@start of free");
    if(!free_list) {
        // The list of free entries is empty, make a new one
        new_entry->size = full_size;
        new_entry->base = (addr_logical_t)hdr;
        new_entry->next = NULL;
        free_list = new_entry;
        free_end = new_entry;
    }else{
        // The list exists!
        kmem_free_t *now = free_list;
        kmem_free_t *prev = NULL;
        for(; now && now->base < (addr_logical_t)hdr; ((prev = now), (now = now->next)));
        _verify("kfree@after get");
        new_entry->size = full_size;
        new_entry->base = (addr_logical_t)hdr;
        new_entry->next = now;
        if(prev) {
            prev->next = new_entry;
        }else{
            free_list = new_entry;
        }
        if(!now) {
            free_end = new_entry;
        }
        _verify("kfree@before merge");
        _merge_free(new_entry);
        if(prev) _merge_free(prev);
    }
    _verify("kfree@end");
    
    memory_used -= hdr->size + sizeof(kmem_header_t);
}

#undef _MINIMUM_PAGES
