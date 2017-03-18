#include <stdint.h>
#include <stddef.h>

#include "mem/page.hpp"
#include "mem/kmem.hpp"
#include "main/printk.hpp"
#include "main/multiboot.hpp"
#include "main/panic.hpp"

namespace page {
    static Page *used_start;
    static Page static_page;
    static int page_id_counter;
    static addr_phys_t allocation_pointer;
    static page_table_entry_t *cursor;
    static multiboot::entry_t *current_map;
    static addr_logical_t virtual_pointer;
    page_dir_t *page_dir;
    static Page *page_free_head;

    typedef struct _empty_virtual_slot_s _empty_virtual_slot_t;
    struct _empty_virtual_slot_s {
        addr_logical_t base;
        unsigned int pages;
        _empty_virtual_slot_t *next;
    };
    static _empty_virtual_slot_t *empty_slot;

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
        Page *now;
        Page *prev = NULL;
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


    static void _merge_free(Page *first) {
        if(first->next && first->mem_base + (first->consecutive * PAGE_SIZE) == first->next->mem_base) {
            Page *hold = first->next;
            first->consecutive += hold->consecutive;
            first->next = hold->next;
            kfree(hold);
            _verify(__func__);
        }
    }


    static void _merge_free_slots(_empty_virtual_slot_t *first) {
        if(first->next && first->base + (first->pages * PAGE_SIZE) == first->next->base) {
            _empty_virtual_slot_t *hold = first->next;
            first->pages += hold->pages;
            first->next = hold->next;
            kfree(hold);
            _verify(__func__);
        }
    }


    void init() {
        page_table_t *page_table;
        
        allocation_pointer = (addr_phys_t)kmem::map.memory_start - KERNEL_VM_BASE;
        
        for(current_map = &(multiboot::mem_table[0]);
            (current_map->base + current_map->length) < allocation_pointer;
            current_map ++);
        
        // The cursor in the table to use
        // This is the logical address of the next unallocated space in the kernel address space
        page_dir = (page_dir_t *)kmem::map.vm_start;
        page_table = (page_table_t *)PAGE_TABLE_NOFLAGS(
            page_dir->entries[kmem::map.vm_end >> PAGE_DIR_SHIFT].table) + KERNEL_VM_BASE;
        cursor = (page_table_entry_t *)((addr_logical_t)(
            &(page_table->entries[((kmem::map.vm_end >> page::PAGE_TABLE_SHIFT) & page::PAGE_TABLE_MASK)]))
                + (addr_logical_t)KERNEL_VM_BASE);
        virtual_pointer = kmem::map.vm_end;
    }


    Page *create(uint32_t base, uint8_t flags, unsigned int count) {
        Page *write;
        
        write = (Page *)kmalloc(sizeof(Page), KMALLOC_RESERVED);
        write->page_id = page_id_counter ++;
        write->mem_base = base;
        write->flags = FLAG_ALLOCATED | flags;
        write->consecutive = count;
        write->next = NULL;
    #if DEBUG_MEM
        printk("Allocated %d pages.\n", count);
    #endif
        
        return write;
    }


    Page *alloc_nokmalloc(uint8_t flags, unsigned int count) {
        Page *write = &static_page;
        unsigned int size = count * PAGE_SIZE;
        
        if((allocation_pointer + size) >= (current_map->base + current_map->length)) {
            size = (current_map->base + current_map->length) - allocation_pointer;
        }
        
        write->page_id = page_id_counter ++;
        write->mem_base = allocation_pointer;
        write->flags = FLAG_ALLOCATED | flags;
        write->consecutive = size / PAGE_SIZE;
        write->next = NULL;
        allocation_pointer = write->mem_base + size;
        
        if((write->mem_base + size) == (current_map->base + current_map->length)) {
    #if DEBUG_MEM
            printk("Have to skip to the next memory region.\n");
    #endif
            current_map ++;
            for(; current_map->type != 1 && current_map < (multiboot::mem_table + sizeof(multiboot::mem_table)); current_map ++);
            if(current_map >= (multiboot::mem_table + sizeof(multiboot::mem_table))) {
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


    Page *alloc(uint8_t flags, unsigned int count) {
        Page *new_page;
        uint8_t alloc_flag = (flags & page::FLAG_RESERVED) ? KMALLOC_RESERVED : 0;
        
        if(count == 0) {
            return NULL;
        }
        
        // Collect all the pages in the free list
        if(page_free_head) {
            if(page_free_head->consecutive > count) {
                new_page = (Page *)kmalloc(sizeof(Page), alloc_flag);
                new_page->page_id = page_id_counter ++;
                new_page->mem_base = page_free_head->mem_base;
                new_page->consecutive = count;
                page_free_head->mem_base += count * PAGE_SIZE;
                page_free_head->consecutive -= count;
            }else{
                new_page = page_free_head;
                page_free_head = new_page->next;
            }
            new_page->flags = flags;
            new_page->next = NULL;
            _verify(__func__);
        }else{
            new_page = (Page *)kmalloc(sizeof(Page), alloc_flag);
            alloc_nokmalloc(flags, count);
            _memcpy(new_page, &static_page, sizeof(Page));
        }
        
        if(new_page->consecutive < count) {
            new_page->next = alloc(flags, count - new_page->consecutive);
        }
        
        return new_page;
    }


    void free(Page *page) {
        Page *now;
        Page *prev = NULL;
        
        if(!page) {
            return;
        }
        
        if(page->next) {
            free(page->next);
        }
        
        for(now = page_free_head; now && now->mem_base < page->mem_base; ((prev = now), (now = now->next)));
        if(prev) {
            prev->next = page;
        }else{
            page_free_head = page;
        }
        page->next = now;
        
        // Try to flatten the free entries
        _merge_free(page);
        if(prev) _merge_free(prev);
    }


    void used(Page *page) {
        Page *old_next = page->next;
        page->next = used_start;
        used_start = page;
        if(old_next) {
            used(old_next);
        }
    }


    void *kinstall_append(Page *page, uint8_t page_flags) {
        uint32_t i;
        addr_logical_t first = 0;
        addr_phys_t base = 0;
        for(i = 0; i < page->consecutive; i ++) {
            if(virtual_pointer >= TOTAL_VM_SIZE - PAGE_SIZE) {
                panic("Ran out of kernel virtual address space!");
            }
            
            cursor->block = (page->mem_base + PAGE_SIZE * i) | page_flags | page::PAGE_TABLE_PRESENT;
            if(!first) {
                first = virtual_pointer;
                base = page->mem_base;
            }
            
            virtual_pointer += PAGE_SIZE;
            
            cursor ++;
        }
        
        kmem::map.memory_end = virtual_pointer;
        
        if(page->next) {
            kinstall_append(page->next, page_flags);
        }
        
    #if DEBUG_MEM
        printk("KInstalled %d new pages at %p onto %p.\n", page->consecutive, base, first);
    #endif
        
        return (void *)first;
    }


    void *kinstall(Page *page, uint8_t page_flags) {
        Page *current = page;
        _empty_virtual_slot_t *slot = empty_slot;
        _empty_virtual_slot_t *prev_slot = NULL;
        unsigned int total_pages = 0;
        addr_logical_t base = 0;
        uint32_t page_offset;
        page_table_entry_t *table_entry;
        unsigned int i;
        
        // Count the total number of pages we need
        total_pages = page->count();
        
        // And search for an empty hole in virtual memory for it
        for(; slot && (slot->pages < total_pages); (prev_slot = slot), (slot = slot->next));
        
        if(slot) {
            if(slot->pages == total_pages) {
                if(prev_slot) {
                    prev_slot->next = slot->next;
                }else{
                    empty_slot = slot->next;
                }
                base = slot->base;
                kfree(slot);
            }else{
                base = slot->base;
                slot->pages -= total_pages;
                slot->base += total_pages * PAGE_SIZE;
            }
            
            if(base) {
                page_offset = (base - KERNEL_VM_BASE) / PAGE_SIZE;
                table_entry = (page_table_entry_t *)(page_dir + 1) + page_offset;
                
                for(current = page; current; current = current->next) {
                    for(i = 0; i < page->consecutive; i ++) {
                        table_entry->block = (current->mem_base + PAGE_SIZE * i) | page_flags | page::PAGE_TABLE_PRESENT;
                        table_entry ++;
                    }
                }
                
                return (void *)base;
            }
        }
        
        // No free spaces could be found
        return kinstall_append(page, page_flags);
    }


    void kuninstall(void *base, Page *page) {
        _empty_virtual_slot_t *now;
        _empty_virtual_slot_t *prev = NULL;
        _empty_virtual_slot_t *new_slot;
        uint32_t page_offset;
        page_table_entry_t *table_entry;
        unsigned int i;
        
        if(!page) {
            return;
        }
        
        for(now = empty_slot; now && now->base < (addr_logical_t)base; ((prev = now), (now = now->next)));
        
        new_slot = (_empty_virtual_slot_t *)kmalloc(sizeof(_empty_virtual_slot_t), 0);
        new_slot->base = (addr_logical_t)base;
        new_slot->pages = page->consecutive;
        
        if(prev) {
            prev->next = new_slot;
        }else{
            empty_slot = new_slot;
        }
        new_slot->next = now;
        
        // Remove the mappings in the page table
        page_offset = ((addr_logical_t)base - KERNEL_VM_BASE) / PAGE_SIZE;
        table_entry = (page_table_entry_t *)(page_dir + 1) + page_offset;
        
        for(i = 0; i < page->consecutive; i ++) {
            table_entry->block = 0;
            table_entry ++;
            __asm__ volatile("invlpg (%0)" ::"r" (base) : "memory");
            base = (void *)((addr_phys_t)base + PAGE_SIZE);
        }
        
        // Try to flatten the free entries
        _merge_free_slots(new_slot);
        if(prev) _merge_free_slots(prev);
        
        if(page->next) {
            kuninstall(base, page->next);
        }
    }


    uint32_t Page::count() {
        uint32_t sum = 0;
        Page *n = this;
        
        for(; n; n = n->next) sum += n->consecutive;
        
        return sum;
    }
}
