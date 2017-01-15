#ifndef __H_MEM_PAGES__
#define __H_MEM_PAGES__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "main/multiboot.h"
#include "main/common.h"

typedef struct page_s page_t;
struct page_s {
    unsigned int page_id;
    addr_phys_t mem_base;
    uint8_t flags;
    unsigned int consecutive;
    page_t *next;
};

#define PAGE_FLAG_ALLOCATED 0x01
#define PAGE_FLAG_KERNEL 0x02
#define PAGE_FLAG_RESERVED 0x04

#define PAGE_FREE_MASK 0xfff

#define PAGE_SIZE 0x1000
#define PAGE_DIR_SIZE 0x400000
#define PAGE_TABLE_LENGTH 0x400

#define PAGE_TABLE_SHIFT 12
#define PAGE_TABLE_MASK 0x3ff
#define PAGE_DIR_SHIFT 22

#define TOTAL_VM_SIZE 0x100000000
#define KERNEL_VM_SIZE 0x40000000
#define KERNEL_VM_PAGES (KERNEL_VM_SIZE / PAGE_SIZE)
#define KERNEL_VM_PAGE_TABLES (KERNEL_VM_PAGES / PAGE_TABLE_LENGTH)
#define KERNEL_VM_BASE (TOTAL_VM_SIZE - KERNEL_VM_SIZE) // Higher half kernel

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

typedef struct page_logical_tables_s {
    page_t *pages[PAGE_TABLE_LENGTH - (KERNEL_VM_PAGE_TABLES)];
    page_table_t *tables[PAGE_TABLE_LENGTH - (KERNEL_VM_PAGE_TABLES)];
} page_logical_tables_t;

#define PAGE_TABLE_NOFLAGS(x) ((x) & ~PAGE_TABLE_FLAGMASK)
#define PAGE_TABLE_FLAGMASK 0xfff
#define PAGE_TABLE_PRESENT 0x01
#define PAGE_TABLE_RW 0x02
#define PAGE_TABLE_USER 0x04
#define PAGE_TABLE_WRITETHROUGH 0x08
#define PAGE_TABLE_CACHEDISABLE 0x10
#define PAGE_TABLE_ACCESSED 0x20
#define PAGE_TABLE_DIRTY 0x40
#define PAGE_TABLE_SIZE 0x80
#define PAGE_TABLE_GLOBAL 0x100

extern page_dir_t *page_dir;

void page_init();
page_t *page_alloc_nokmalloc(uint8_t flags, unsigned int count);
page_t *page_alloc(uint8_t flags, unsigned int count);
page_t *page_create(uint32_t base, uint8_t flags, unsigned int count);
void page_free(page_t *page);
void page_used(page_t *page);
void *page_kinstall(page_t *page, uint8_t page_flags);
void *page_kinstall_append(page_t *page, uint8_t page_flags); // Doesn't kmalloc, but doesn't reuse any existing memory
// either
void page_kuninstall(void *base, page_t *page);
uint32_t page_count(page_t *page);

#endif
