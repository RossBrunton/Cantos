#ifndef __HPP_MEM_PAGES__
#define __HPP_MEM_PAGES__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "main/multiboot.hpp"
#include "main/common.h"

namespace page {
    class Page {
    public:
        unsigned int page_id;
        addr_phys_t mem_base;
        uint8_t flags;
        unsigned int consecutive;
        Page *next;
        
        uint32_t count();
        Page *split(uint32_t count);
    };

    const uint8_t FLAG_ALLOCATED = 0x01;
    const uint8_t FLAG_KERNEL = 0x02;
    const uint8_t FLAG_RESERVED = 0x04;

    const uint32_t FREE_MASK = 0xfff;

    const uint32_t PAGE_TABLE_SHIFT = 12;
    const uint32_t PAGE_TABLE_MASK = 0x3ff;
    const uint32_t PAGE_DIR_SHIFT = 22;

    // Page table stuff
    typedef struct page_table_entry_s {
        uint32_t block;
    } page_table_entry_t;

    typedef struct page_table_s {
        page_table_entry_t entries[PAGE_TABLE_LENGTH];
    } page_table_t;

    typedef struct page_dir_entry_s {
        uint32_t table;
    } page_dir_entry_t;

    typedef struct page_dir_s {
        page_dir_entry_t entries[PAGE_TABLE_LENGTH];
    } page_dir_t;

    typedef struct logical_tables_s {
        Page *pages[PAGE_TABLE_LENGTH - (KERNEL_VM_PAGE_TABLES)];
        page_table_t *tables[PAGE_TABLE_LENGTH - (KERNEL_VM_PAGE_TABLES)];
    } logical_tables_t;

    #define PAGE_TABLE_NOFLAGS(x) ((x) & ~PAGE_TABLE_FLAGMASK)
    const uint32_t PAGE_TABLE_FLAGMASK = 0xfff;
    const uint32_t PAGE_TABLE_PRESENT = 0x01;
    const uint32_t PAGE_TABLE_RW = 0x02;
    const uint32_t PAGE_TABLE_USER = 0x04;
    const uint32_t PAGE_TABLE_WRITETHROUGH = 0x08;
    const uint32_t PAGE_TABLE_CACHEDISABLE = 0x10;
    const uint32_t PAGE_TABLE_ACCESSED = 0x20;
    const uint32_t PAGE_TABLE_DIRTY = 0x40;
    const uint32_t PAGE_TABLE_SIZE = 0x80;
    const uint32_t PAGE_TABLE_GLOBAL = 0x100;

    extern page_dir_t *page_dir;

    void init();
    Page *alloc(uint8_t flags, unsigned int count);
    Page *alloc_nokmalloc(uint8_t flags, unsigned int count);
    Page *create(uint32_t base, uint8_t flags, unsigned int count);
    void free(Page *page);
    void used(Page *page);
    void *kinstall(Page *page, uint8_t page_flags);
    void *kinstall_append(Page *page, uint8_t page_flags); // Doesn't kmalloc, but doesn't reuse any existing memory
    // either
    void kuninstall(void *base, Page *page);
    
}

#endif
