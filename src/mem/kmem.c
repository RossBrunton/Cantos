#include "main/printk.h"
#include "mem/kmem.h"
#include "mem/page.h"

static page_t *kernel_start;

static kmem_free_t *free_list;

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
    size_t size_needed;
    kmem_header_t *hdr;
    
    if(size == 0) return NULL;
    
    // Align size to four bytes
    size += 4 - size % 4;
    
    size_needed = size + sizeof(kmem_header_t);
    printk("Allocating %x\n", size);
    
    for(; free; (free = free->next)) {
        if(size_needed < free->size) {
            // Do it!
            if(size_needed + sizeof(kmem_header_t) >= free->size) {
                // Not a typo: If the remaining space is not enough to store any headers, just use up the whole block
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
    
    return NULL;
}

void kfree(void *ptr) {
    
}
