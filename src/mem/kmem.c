#include "main/printk.h"
#include "mem/kmem.h"
#include "mem/page.h"

static page_t *kernel_start;
static kmem_free_t *free_list;
static kmem_free_t *free_free_structs;

static void *_memcpy(void *destination, const void *source, size_t num) {
    size_t i;
    for(i = 0; i < num; i ++) {
        ((char *)destination)[i] = ((char *)source)[i];
    }
    return destination;
}


void kmem_init(multiboot_info_t *mbi) {
    page_t *initial;
    kmem_header_t header;
    kmem_free_t free_block;
    ptrdiff_t end_pointer = 0;
    
    initial = page_init(mbi);
    
    // Memory header for the page header
    header.size = sizeof(page_t);
    _memcpy(initial->mem_base, &header, sizeof(kmem_header_t));
    end_pointer += sizeof(kmem_header_t);
    
    // And the struct
    _memcpy(initial->mem_base+end_pointer, initial, sizeof(page_t));
    kernel_start = initial->mem_base+end_pointer;
    end_pointer += sizeof(page_t);
    
    // And now for the initial free block thing's header
    header.size = sizeof(kmem_free_t);
    _memcpy(initial->mem_base+end_pointer, &header, sizeof(kmem_header_t));
    end_pointer += sizeof(kmem_header_t);
    
    // And its value
    free_block.size = PAGE_SIZE - end_pointer - sizeof(kmem_free_t);
    free_block.base = initial->mem_base + end_pointer + sizeof(kmem_free_t);
    free_block.next = NULL;
    _memcpy(initial->mem_base+end_pointer, &free_block, sizeof(kmem_free_t));
    free_list = initial->mem_base+end_pointer;
    
    printk("Allocations: kernel_start: %p (%x) free_block: %p (%x)\n", kernel_start, sizeof(page_t), free_list, sizeof(kmem_free_t));
    printk("Free memory starts from %p with size %x\n", free_list->base, free_list->size);
}


void *kmalloc(size_t size) {
    kmem_free_t *free = free_list;
    kmem_free_t *prev = NULL;
    size_t size_needed = 0;
    kmem_header_t *hdr = NULL;
    
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
                
                // Update the free list
                size = free->size - sizeof(kmem_header_t);
                if(!prev) {
                    free_list = free->next;
                }else{
                    prev->next = free->next;
                }
                base = free->base;
                
                // And mix the structure into the free_free_structs
                free->next = free_free_structs;
                free_free_structs = free;
                
                // And then do that malloc thing we were going to do
                hdr = base;
                hdr->size = size;
                return base + sizeof(kmem_header_t);
            }else{
                // Otherwise just shrink it
                void *base = free->base;
                free->size -= size_needed;
                free->base += size_needed;
                hdr = base;
                hdr->size = size;
                return base + sizeof(kmem_header_t);
            }
        }
    }
    
    kerror("kmalloc failure!\n");
    return NULL;
}


static kmem_free_t *_get_struct() {
    kmem_free_t *hold;
    if(free_free_structs) {
        hold = free_free_structs;
        free_free_structs = free_free_structs->next;
        return hold;
    }else{
        hold = kmalloc(sizeof(kmem_free_t));
        return hold;
    }
}


inline static void _print() {
    kmem_free_t *now;
    for(now = free_list; now; now = now->next) {
        printk("[%p %d/%x] ", now->base, now->size, now->size);
    }
    printk("\n");
}


static void _merge_free(kmem_free_t *first) {
    if(first->next && first->base + first->size == first->next->base) {
        kmem_free_t *hold = first->next;
        first->size += hold->size;
        first->next = hold->next;
        hold->next = free_free_structs;
        free_free_structs = hold;
    }
}


void kfree(void *ptr) {
    kmem_header_t *hdr = ptr - sizeof(kmem_header_t);
    size_t full_size = hdr->size + sizeof(kmem_header_t);
    if(!free_list) {
        // The list of free entries is empty, make a new one
        kmem_free_t *new_entry = _get_struct();
        new_entry->size = full_size;
        new_entry->base = hdr;
        new_entry->next = NULL;
        free_list = new_entry;
    }else{
        // The list exists!
        kmem_free_t *now = free_list;
        kmem_free_t *prev = NULL;
        for(; now->next && now->next->base > (void *)hdr; ((prev = now), (now = now->next)));
        kmem_free_t *new_entry = _get_struct();
        new_entry->size = full_size;
        new_entry->base = hdr;
        new_entry->next = now;
        if(prev) {
            prev->next = new_entry;
        }else{
            free_list = new_entry;
        }
        
        _merge_free(new_entry);
        _merge_free(prev);
    }
}
